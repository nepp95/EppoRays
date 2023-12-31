#pragma once

#include <glm/glm/glm.hpp>

#include <memory>

class Renderer
{
public:
	Renderer() = default;

	void Init();

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Camera& camera);

	const std::shared_ptr<Eppo::Image>& GetImage() const { return m_Image; }
	uint32_t* GetImageData() const { return m_ImageData; }

	uint32_t GetViewportWidth() const { return m_ViewportWidth; }
	uint32_t GetViewportHeight() const { return m_ViewportHeight; }

private:
	glm::vec4 TraceRay(const Ray& ray) const;

private:
	std::shared_ptr<Eppo::Image> m_Image;
	uint32_t* m_ImageData = nullptr;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
};
