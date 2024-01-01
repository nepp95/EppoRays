#include "AppLayer.h"

#include <imgui/imgui.h>

void AppLayer::OnAttach()
{
}

void AppLayer::OnUpdate(float timestep)
{
	Timer timer;

	m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
	m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);

	m_Camera.OnUpdate(timestep);
	m_Renderer.Render(m_Camera);

	m_LastRenderTime = timer.GetElapsedMicroseconds();
}

void AppLayer::OnUIRender()
{
	// Viewport
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");

	m_ViewportWidth = ImGui::GetContentRegionAvail().x;
	m_ViewportHeight = ImGui::GetContentRegionAvail().y;

	const auto& image = m_Renderer.GetImage();
	if (image)
		ImGui::Image((ImTextureID)image->GetRendererID(), ImVec2(image->GetWidth(), image->GetHeight()), ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Settings");
	ImGui::Text("Render time: %.3fms", m_LastRenderTime / 1000.0f);
	ImGui::End();
}
