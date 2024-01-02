#pragma once

#include <glm/glm.hpp>

#include <vector>

class Camera
{
public:
	Camera(float verticalFOV, float nearClip, float farClip);

	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	void SetPosition(const glm::vec3& position);
	void SetDirection(const glm::vec3& direction);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }

	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }

	const glm::vec3& GetRayDirection(uint32_t x, uint32_t y) const { return m_RayDirections[y * m_ViewportWidth + x]; }

	float GetRotationSpeed() const;

private:
	void CalculateProjection();
	void CalculateView();
	void CacheRayDirections();

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

	std::vector<glm::vec3> m_RayDirections;
};
