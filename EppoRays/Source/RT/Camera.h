#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	Camera(float verticalFOV, float nearClip, float farClip);

	void OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetView() const { return m_View; }

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }

	float GetRotationSpeed() const;

private:
	void CalculateProjection();
	void CalculateView();

private:
	glm::mat4 m_Projection = glm::mat4(1.0f);
	glm::mat4 m_InverseProjection = glm::mat4(1.0f);

	glm::mat4 m_View = glm::mat4(1.0f);
	glm::mat4 m_InverseView = glm::mat4(1.0f);

	float m_VerticalFOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 1000.0f;

	glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_ForwardDirection = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec2 m_LastMousePosition = glm::vec2(0.0f, 0.0f);
	uint32_t m_ViewportWidth = 0;
	uint32_t m_ViewportHeight = 0;
};
