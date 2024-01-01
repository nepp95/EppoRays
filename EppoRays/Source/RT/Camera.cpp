#include "Camera.h"

#include <EppoCore/Core/Input.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace Eppo;

Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: m_VerticalFOV(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip)
{
	m_ForwardDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	m_Position = glm::vec3(0.0f, 1.0f, 7.0f);
}

bool Camera::OnUpdate(float ts)
{
	// Calculate the mouse movement delta
	glm::vec2 mousePosition = Input::GetMousePosition();
	glm::vec2 delta = (mousePosition - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePosition;

	// Reset the mouse cursor to act normal if we are not moving the camera
	if (!Input::IsMouseButtonPressed(Mouse::ButtonRight))
	{
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}

	// We don't want the mouse cursor to move all over the place when moving the camera
	Input::SetCursorMode(CursorMode::Locked);

	// Movement
	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

	float speed = 2.0f;
	bool moved = false;

	// Fwd/Bwd
	if (Input::IsKeyPressed(Key::W))
	{
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyPressed(Key::S))
	{
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}
	
	// Left/Right
	if (Input::IsKeyPressed(Key::A))
	{
		m_Position -= rightDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyPressed(Key::D))
	{
		m_Position += rightDirection * speed * ts;
		moved = true;
	}

	// Up/Down
	if (Input::IsKeyPressed(Key::E))
	{
		m_Position += upDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyPressed(Key::Q))
	{
		m_Position -= upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.0f, 1.0f, 0.0f))));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved)
	{
		CalculateView();
		CacheRayDirections();
	}

	return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height)
{
	if (width == m_ViewportWidth && height == m_ViewportHeight)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	CalculateProjection();
	CacheRayDirections();
}

float Camera::GetRotationSpeed() const
{
	return 0.3f;
}

void Camera::CalculateProjection()
{
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::CalculateView()
{
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0.0f, 1.0f, 0.0f));
	m_InverseView = glm::inverse(m_View);
}

void Camera::CacheRayDirections()
{
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++)
	{
		for (uint32_t x = 0; x < m_ViewportWidth; x++)
		{
			// Calculate normalized device coordinates (-1 to 1)
			glm::vec2 coord = { (float)x / m_ViewportWidth, (float)y / m_ViewportHeight };
			coord = coord * 2.0f - 1.0f;

			// Take our camera into account
			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1.0f, 1.0f);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0.0f));

			m_RayDirections[y * m_ViewportWidth + x] = rayDirection;
		}
	}
}
