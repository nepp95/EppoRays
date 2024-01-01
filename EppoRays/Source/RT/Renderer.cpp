#include "Renderer.h"

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

	m_ViewportWidth = width;
	m_ViewportHeight = height;
}

void Renderer::Render(const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	uint32_t width = m_Image->GetWidth();
	uint32_t height = m_Image->GetHeight();

	for (uint32_t y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			// Calculate normalized device coordinates (-1 to 1)
			glm::vec2 coord = { (float)x / width, (float)y / height };
			coord = coord * 2.0f - 1.0f;

			ray.Direction = glm::vec3(coord, -1.0f);
			glm::vec4 color = TraceRay(ray);

			m_ImageData[y * width + x] = Utils::ConvertToRGBA(color);
		}
	}

	m_Image->SetData(m_ImageData, width * height);
}

glm::vec4 Renderer::TraceRay(const Ray& ray) const
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
		return glm::vec4(0.0f);

	// Quadratic formula:
	// (-b +- sqrt(discriminant)) / 2a
	float sqrtD = glm::sqrt(discriminant);

	float closestT = (-b - sqrtD) / (2.0f * a);
	float t1 = (-b + sqrtD) / (2.0f * a);

	glm::vec3 hitPoint = ray.Origin + ray.Direction * closestT;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	float lightIntensity = glm::max(glm::dot(normal, -lightDirection), 0.0f);

	glm::vec3 color(0.0f, 1.0f, 0.0f);
	color *= lightIntensity;

	return glm::vec4(color, 1.0f);
}
