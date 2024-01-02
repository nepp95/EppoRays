#include "Renderer.h"

#include <EppoCore/Core/Random.h>

#include <execution>

namespace Utils
{
	inline static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint32_t r = (uint8_t)(color.r * 255.0f);
		uint32_t g = (uint8_t)(color.g * 255.0f);
		uint32_t b = (uint8_t)(color.b * 255.0f);
		uint32_t a = (uint8_t)(color.a * 255.0f);

		return (a << 24) | (b << 16) | (g << 8) | r;
	}

	inline static glm::vec3 Lerp(const glm::vec3& startValue, const glm::vec3& endValue, float value)
	{
		// blendedValue = (1 - a) * start + a * end
		float a = 0.5f * (value + 1.0f);

		return (1.0f - a) * startValue + a * endValue;
	}
}

void Renderer::Init()
{
	m_Compute = std::make_shared<Eppo::ComputeShader>("Shaders/rt.glsl");

	m_CameraUB = std::make_shared<Eppo::UniformBuffer>(sizeof(CameraData), 3);
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_Image && width == m_Image->GetWidth() && height == m_Image->GetHeight())
		return;

	m_Image = std::make_shared<Eppo::Image>(width, height);
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulatedColorData;
	m_AccumulatedColorData = new glm::vec3[width * height];
	m_FrameIndex = 1;

	m_PixelSB = std::make_shared<Eppo::Buffer>(width * height * sizeof(glm::vec4), 0);

	m_VerticalIterator.resize(height);
	for (uint32_t i = 0; i < height; i++)
		m_VerticalIterator[i] = i;
		
	m_ViewportWidth = width;
	m_ViewportHeight = height;
}

void Renderer::Render(const Scene& scene, const Camera& camera, RenderMode mode)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
		memset(m_AccumulatedColorData, 0, m_Image->GetWidth() * m_Image->GetHeight() * sizeof(glm::vec3));

	switch (mode)
	{
		case RenderMode::CpuST: RenderST(); break;
		case RenderMode::CpuMT: RenderMT(); break;
		case RenderMode::Gpu:	RenderGPU(); break;
	}

	m_Image->SetData(m_ImageData, m_Image->GetWidth() * m_Image->GetHeight());

	if (m_Settings.m_Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}

void Renderer::RenderCommon(uint32_t y)
{
	for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
	{
		glm::vec3 color = RayGen(x, y);
		m_AccumulatedColorData[y * m_Image->GetWidth() + x] += color;

		glm::vec3 accumulatedColor = m_AccumulatedColorData[y * m_Image->GetWidth() + x];
		accumulatedColor /= (float)m_FrameIndex;
		accumulatedColor = glm::clamp(accumulatedColor, 0.0f, 1.0f);

		m_ImageData[y * m_Image->GetWidth() + x] = Utils::ConvertToRGBA(glm::vec4(accumulatedColor, 1.0f));
	}
}

void Renderer::RenderST()
{
	for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
	{
		RenderCommon(y);
	}
}

void Renderer::RenderMT()
{
	std::for_each(std::execution::par, m_VerticalIterator.begin(), m_VerticalIterator.end(), [this](uint32_t y)
	{
		RenderCommon(y);
	});
}

void Renderer::RenderGPU()
{
	// Update camera uniform
	m_CameraData.View = m_ActiveCamera->GetView();
	m_CameraData.InverseView = m_ActiveCamera->GetInverseView();
	m_CameraData.Projection = m_ActiveCamera->GetProjection();
	m_CameraData.InverseProjection = m_ActiveCamera->GetInverseProjection();
	m_CameraData.Position = glm::vec4(m_ActiveCamera->GetPosition(), (float)m_FrameIndex);
	m_CameraData.Direction = glm::vec4(m_ActiveCamera->GetDirection(), 0.0);

	m_CameraUB->SetData(&m_CameraData, sizeof(CameraData));

	// Update sphere storage
	{
		uint32_t size = m_ActiveScene->m_Spheres.size() * sizeof(Sphere);
		if (!m_SphereSB || size != m_SphereSB->GetSize())
			m_SphereSB = std::make_shared<Eppo::Buffer>(size, 1);

		m_SphereSB->SetData((void*)m_ActiveScene->m_Spheres.data(), size);
	}

	// Update material storage
	{
		uint32_t size = m_ActiveScene->m_Materials.size() * sizeof(Material);
		if (!m_MaterialSB || size != m_MaterialSB->GetSize())
			m_MaterialSB = std::make_shared<Eppo::Buffer>(size, 2);

		m_MaterialSB->SetData((void*)m_ActiveScene->m_Materials.data(), size);
	}

	// Dispatch compute shader
	Eppo::Query query;
	query.Begin();

	m_Compute->Bind();
	m_Compute->Dispatch(m_Image->GetWidth(), m_Image->GetHeight(), 1);

	query.End();
	m_Settings.m_LastRenderTime = query.GetResults();

	m_Compute->MemBarrier();

	glm::vec4* data = m_PixelSB->MapBuffer();
	if (!data)
		return;

	for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
		{
			glm::vec4 color = data[y * m_Image->GetWidth() + x];
			m_AccumulatedColorData[y * m_Image->GetWidth() + x] += glm::vec3(color.r, color.g, color.b);

			glm::vec3 accumulatedColor = m_AccumulatedColorData[y * m_Image->GetWidth() + x];
			accumulatedColor /= (float)m_FrameIndex;
			accumulatedColor = glm::clamp(accumulatedColor, 0.0f, 1.0f);

			m_ImageData[y * m_Image->GetWidth() + x] = Utils::ConvertToRGBA(glm::vec4(accumulatedColor, 1.0f));
		}
	}

	m_PixelSB->UnmapBuffer();
}

glm::vec3 Renderer::RayGen(uint32_t x, uint32_t y) const
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirection(x, y);

	glm::vec3 light(0.0f);
	glm::vec3 contribution(1.0f);

	uint32_t seed = y * m_Image->GetWidth() + x;
	seed *= m_FrameIndex;

	uint32_t bounces = 10;
	for (uint32_t i = 0; i < bounces; i++)
	{
		seed += i;

		HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor = Utils::Lerp(glm::vec3(1.0f), glm::vec3(0.5f, 0.7f, 1.0f), ray.Direction.y);
			//color += skyColor * multiplier;
			break;
		}

		const Sphere& sphere = m_ActiveScene->m_Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->m_Materials[sphere.MaterialIndex];

		contribution *= material.Albedo;
		light += material.Emission * material.EmissionPower;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::normalize(payload.WorldNormal + Eppo::FastRandom::InUnitSphere(seed));
	}
	
	return light;
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) const
{
	int closestSphere = -1;
	float closestHit = FLT_MAX;

	for (size_t i = 0; i < m_ActiveScene->m_Spheres.size(); i++)
	{
		const auto& sphere = m_ActiveScene->m_Spheres[i];

		glm::vec3 origin = ray.Origin - sphere.Position;

		// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
		// where
		// a = ray origin
		// b = ray direction
		// r = radius
		// t = hit distance
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// Quadratic forumula discriminant:
		// b^2 - 4ac
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0)
			continue;

		// Quadratic formula:
		// (-b +- sqrt(discriminant)) / 2a
		
		//float t1 = (-b + sqrtD) / (2.0f * a);
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		if (closestT < closestHit && closestT > 0.0f)
		{
			closestHit = closestT;
			closestSphere = i;
		}
	}

	if (closestSphere < 0)
		return Miss();

	return ClosestHit(ray, closestHit, closestSphere);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, uint32_t objectIndex) const
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->m_Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss() const
{
	return Renderer::HitPayload();
}
