#pragma once

#include <EppoCore.h>
#include "RT/Camera.h"
#include "RT/Ray.h"
#include "RT/Scene.h"

#include <glm/glm.hpp>

#include <memory>

class Renderer
{
public:
	enum class RenderMode
	{
		CpuST,
		CpuMT,
		Gpu
	};

	struct Settings
	{
		bool Accumulate = true;
		RenderMode Mode;
		uint64_t LastRenderTime = 0;
	};

public:
	Renderer() = default;

	void Init();

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera, RenderMode mode);

	Settings& GetSettings() { return m_Settings; }

	uint32_t GetFrameIndex() const { return m_FrameIndex; }
	void ResetFrameIndex() { m_FrameIndex = 1; }

	const std::shared_ptr<Eppo::Image>& GetImage() const { return m_Image; }
	uint32_t* GetImageData() const { return m_ImageData; }

	uint32_t GetViewportWidth() const { return m_ViewportWidth; }
	uint32_t GetViewportHeight() const { return m_ViewportHeight; }

private:
	struct HitPayload
	{
		float HitDistance = -1.0f;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		uint32_t ObjectIndex;
	};

	void RenderCommon(uint32_t y);
	void RenderST();
	void RenderMT();
	void RenderGPU();

	glm::vec3 RayGen(uint32_t x, uint32_t y) const;

	HitPayload TraceRay(const Ray& ray) const;
	HitPayload ClosestHit(const Ray& ray, float hitDistance, uint32_t objectIndex) const;
	HitPayload Miss() const;

private:
	Settings m_Settings;

	std::shared_ptr<Eppo::ComputeShader> m_Compute;
	std::shared_ptr<Eppo::Buffer> m_PixelSB;
	std::shared_ptr<Eppo::Buffer> m_SphereSB;
	std::shared_ptr<Eppo::Buffer> m_MaterialSB;

	std::shared_ptr<Eppo::Image> m_Image;
	uint32_t* m_ImageData = nullptr;

	struct CameraData
	{
		glm::mat4 View;
		glm::mat4 InverseView;
		glm::mat4 Projection;
		glm::mat4 InverseProjection;
		glm::vec4 Position;
		glm::vec4 Direction;
	} m_CameraData;
	std::shared_ptr<Eppo::UniformBuffer> m_CameraUB;

	glm::vec3* m_AccumulatedColorData = nullptr;
	uint32_t m_FrameIndex = 1;

	const Camera* m_ActiveCamera = nullptr;
	const Scene* m_ActiveScene = nullptr;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	std::vector<uint32_t> m_VerticalIterator;
};
