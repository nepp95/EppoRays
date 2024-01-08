#include "AppLayer.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>

void AppLayer::OnAttach()
{
	{
		Material& material = m_Scene.m_Materials.emplace_back();
		material.Albedo = glm::vec3(0.2f, 1.0f, 0.2f);
		material.Roughness = 0.02f;
	}

	{
		Material& material = m_Scene.m_Materials.emplace_back();
		material.Albedo = glm::vec3(0.2f, 0.6f, 0.8f);
		material.Roughness = 0.4f;
	}

	{
		Material& material = m_Scene.m_Materials.emplace_back();
		material.Albedo = glm::vec3(1.0f, 1.0f, 1.0f);
		material.Roughness = 1.00f;
		material.Emission = glm::vec3(1.0f, 1.0f, 1.0f);
		material.EmissionPower = 2.0f;
	}

	{
		Sphere& sphere = m_Scene.m_Spheres.emplace_back();
		sphere.Position = glm::vec3(0.0f, 1.0f, 0.0f);
		sphere.Radius = 1.0f;
		sphere.MaterialIndex = 0;
	}

	{
		Sphere& sphere = m_Scene.m_Spheres.emplace_back();
		sphere.Position = glm::vec3(0.0f, -50.0f, 0.0f);
		sphere.Radius = 50.0f;
		sphere.MaterialIndex = 1;
	}

	{
		Sphere& sphere = m_Scene.m_Spheres.emplace_back();
		sphere.Position = glm::vec3(0.0f, 150.0f, 0.0f);
		sphere.Radius = 100.0f;
		sphere.MaterialIndex = 2;
	}

	{
		Sphere& sphere = m_Scene.m_Spheres.emplace_back();
		sphere.Position = glm::vec3(-3.0f, 1.5f, 0.0f);
		sphere.Radius = 1.5f;
		sphere.MaterialIndex = 0;
	}

	for (uint32_t i = 0; i < 5; i++)
	{
		Sphere& sphere = m_Scene.m_Spheres.emplace_back();
		sphere.Position = glm::vec3(1.0f - i, 0.3f, -cos(-2.0f + (float)i) - 5.0f);
		sphere.Radius = 0.5f;
		sphere.MaterialIndex = Eppo::Random::UInt32(0, 1);
	}

	m_Camera.SetPosition(glm::vec3(5.9f, 6.5f, -0.3f));
	m_Camera.SetDirection(glm::vec3(-0.8f, -0.6f, -0.2f));

	m_Renderer.Init();
}

void AppLayer::OnUpdate(float timestep)
{
	m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
	m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);

	if (m_Camera.OnUpdate(timestep))
		m_Renderer.ResetFrameIndex();

	Timer renderTimer;
	m_Renderer.Render(m_Scene, m_Camera, Renderer::RenderMode::CpuMT);
	m_LastRenderTime = renderTimer.GetElapsedMicroseconds();
}

void AppLayer::OnUIRender()
{
	// Viewport
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");

	m_ViewportWidth = (uint32_t)ImGui::GetContentRegionAvail().x;
	m_ViewportHeight = (uint32_t)ImGui::GetContentRegionAvail().y;

	const auto& image = m_Renderer.GetImage();
	if (image)
		ImGui::Image((ImTextureID)image->GetRendererID(), ImVec2((float)image->GetWidth(), (float)image->GetHeight()), ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Settings");

	auto& settings = m_Renderer.GetSettings();
	
	ImGui::Text("Render time(cpu): %.3fms", m_LastRenderTime / 1000.0f);

	if (settings.Mode == Renderer::RenderMode::Gpu)
		ImGui::Text("Render time(gpu): %.3fms", settings.LastRenderTime / 1000.0f / 1000.0f);

	ImGui::Text("Camera position: X: %.1f, Y: %.1f, Z: %.1f", m_Camera.GetPosition().x, m_Camera.GetPosition().y, m_Camera.GetPosition().z);
	ImGui::Text("Camera direction: X: %.1f, Y: %.1f, Z: %.1f", m_Camera.GetDirection().x, m_Camera.GetDirection().y, m_Camera.GetDirection().z);

	ImGui::Checkbox("Accumulate", &settings.Accumulate);

	bool accumulate = m_Renderer.GetSettings().Accumulate;
	if (accumulate)
		ImGui::Text("Frames accumulated: %d", m_Renderer.GetFrameIndex());
	
	if (ImGui::Button("Reset"))
		m_Renderer.ResetFrameIndex();

	ImGui::Separator();

	bool changed = false;
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen))
	{
		for (uint32_t i = 0; i < m_Scene.m_Materials.size(); i++)
		{
			Material& material = m_Scene.m_Materials[i];

			if (i > 0)
				ImGui::Separator();

			ImGui::PushID(i);
			if (ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo))) changed = true;
			if (ImGui::ColorEdit3("Emission", glm::value_ptr(material.Emission))) changed = true;
			if (ImGui::DragFloat("Emission Power", &material.EmissionPower, 0.01f, 0.0f, 1000.0f)) changed = true;
			if (ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f)) changed = true;
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
			if (ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f)) changed = true;
			if (ImGui::DragFloat("Radius", &sphere.Radius)) changed = true;
			if (ImGui::DragInt("Material index", (int*)&sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.m_Materials.size() - 1)) changed = true;
			ImGui::PopID();
		}
	}

	if (changed)
		m_Renderer.ResetFrameIndex();

	ImGui::End();
}
