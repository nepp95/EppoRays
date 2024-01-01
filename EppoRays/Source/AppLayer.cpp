#include "AppLayer.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

void AppLayer::OnAttach()
{
	Material& greenMaterial = m_Scene.m_Materials.emplace_back();
	greenMaterial.Albedo = glm::vec3(0.2f, 1.0f, 0.2f);
	greenMaterial.Roughness = 0.05f;

	Material& blueMaterial = m_Scene.m_Materials.emplace_back();
	blueMaterial.Albedo = glm::vec3(0.2f, 0.6f, 0.8f);
	blueMaterial.Roughness = 0.1f;

	Sphere& smallSphere = m_Scene.m_Spheres.emplace_back();
	smallSphere.Position = glm::vec3(0.0f, 1.0f, 0.0f);
	smallSphere.Radius = 1.0f;
	smallSphere.MaterialIndex = 0;

	Sphere& bigSphere = m_Scene.m_Spheres.emplace_back();
	bigSphere.Position = glm::vec3(0.0f, -50.0f, 0.0f);
	bigSphere.Radius = 50.0f;
	bigSphere.MaterialIndex = 1;
}

void AppLayer::OnUpdate(float timestep)
{
	m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
	m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);

	if (m_Camera.OnUpdate(timestep))
		m_Renderer.ResetFrameIndex();

	Timer renderTimer;
	m_Renderer.Render(m_Scene, m_Camera);
	m_LastRenderTime = renderTimer.GetElapsedMicroseconds();
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
	ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().m_Accumulate);
	
	if (ImGui::Button("Reset"))
		m_Renderer.ResetFrameIndex();

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (uint32_t i = 0; i < m_Scene.m_Materials.size(); i++)
		{
			Material& material = m_Scene.m_Materials[i];

			if (i > 0)
				ImGui::Separator();

			ImGui::PushID(i);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Spheres", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (uint32_t i = 0; i < m_Scene.m_Spheres.size(); i++)
		{
			Sphere& sphere = m_Scene.m_Spheres[i];

			if (i > 0)
				ImGui::Separator();

			ImGui::PushID(i);
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position));
			ImGui::DragFloat("Radius", &sphere.Radius);
			ImGui::DragInt("Material index", (int*)&sphere.MaterialIndex, 1.0f, 0, m_Scene.m_Materials.size() - 1);
			ImGui::PopID();
		}
	}

	ImGui::End();
}
