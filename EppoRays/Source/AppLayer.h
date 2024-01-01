#pragma once

#include <EppoCore.h>
#include "RT/Camera.h"
#include "RT/Renderer.h"

using namespace Eppo;

class AppLayer : public Layer
{
public:
	AppLayer() = default;
	~AppLayer() override = default;

	void OnAttach() override;

	void OnUpdate(float timestep) override;
	void OnUIRender() override;

private:
	Camera m_Camera = Camera(45.0f, 0.1f, 1000.0f);
	Scene m_Scene;
	Renderer m_Renderer;

	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;

	uint64_t m_LastRenderTime = 0;
};
