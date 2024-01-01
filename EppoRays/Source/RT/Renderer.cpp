#include "Renderer.h"

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

	inline static glm::vec4 Lerp(const glm::vec4& startValue, const glm::vec4& endValue, float value)
	{
		// blendedValue = (1 - a) * start + a * end
		float a = 0.5f * (value + 1.0f);

		return (1.0f - a) * startValue + a * endValue;
	}
}

void Renderer::Init()
{

}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_Image && width == m_Image->GetWidth() && height == m_Image->GetHeight())
		return;

	m_Image = std::make_shared<Eppo::Image>(width, height);
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	m_HorizontalIterator.resize(width);
	for (uint32_t i = 0; i < width; i++)
		m_HorizontalIterator[i] = 1;

	m_VerticalIterator.resize(height);
	for (uint32_t i = 0; i < height; i++)
		m_VerticalIterator[i] = 1;
		
	m_ViewportWidth = width;
	m_ViewportHeight = height;
}

void Renderer::Render(const Camera& camera)
{
	m_ActiveCamera = &camera;

#define MT 0
#if MT

	std::for_each(std::execution::par, m_VerticalIterator.begin(), m_VerticalIterator.end(), [this](uint32_t y)
	{
		std::for_each(std::execution::par, m_HorizontalIterator.begin(), m_HorizontalIterator.end(), [this, y](uint32_t x)
		{
			glm::vec4 color = RayGen(x, y);
			m_ImageData[y * m_Image->GetWidth() + x] = Utils::ConvertToRGBA(color);
		});
	});

#else

	for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
		{
			glm::vec4 color = RayGen(x, y);
			m_ImageData[y * m_Image->GetWidth() + x] = Utils::ConvertToRGBA(color);
		}
	}

#endif

	m_Image->SetData(m_ImageData, m_Image->GetWidth() * m_Image->GetHeight());
}

glm::vec4 Renderer::RayGen(uint32_t x, uint32_t y) const
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirection(x, y);

	HitPayload payload = TraceRay(ray);

	if (payload.HitDistance < 0.0f)
		return glm::vec4(0.0f);

	glm::vec3 color(0.0f, 1.0f, 0.0f);

	glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDirection), 0.0f);

	color *= lightIntensity;

	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) const
{
	float radius = 0.5f;

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance
	float a = glm::dot(ray.Direction, ray.Direction);
	float b = 2.0f * glm::dot(ray.Origin, ray.Direction);
	float c = glm::dot(ray.Origin, ray.Origin) - radius * radius;

	// Quadratic forumula discriminant:
	// b^2 - 4ac
	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0)
		return Miss();

	// Quadratic formula:
	// (-b +- sqrt(discriminant)) / 2a
	float sqrtD = glm::sqrt(discriminant);

	float closestT = (-b - sqrtD) / (2.0f * a);
	//float t1 = (-b + sqrtD) / (2.0f * a);

	return ClosestHit(ray, closestT);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance) const
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.WorldPosition = ray.Origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	return payload;
}

Renderer::HitPayload Renderer::Miss() const
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;

	return payload;
}
