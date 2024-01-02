#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Sphere
{
	glm::vec3 Position = glm::vec3(0.0f);
	float Radius = 1.0f;

	uint32_t MaterialIndex = 0;
};

struct Material
{
	glm::vec3 Albedo = glm::vec3(1.0f);
	float Roughness = 0.0f;

	glm::vec3 Emission = glm::vec3(1.0f, 1.0f, 1.0f);
	float EmissionPower = 0.0f;
};

struct Scene
{
	std::vector<Sphere> m_Spheres;
	std::vector<Material> m_Materials;
};
