#pragma once

#include "glm/glm.hpp"

struct Ray
{
	glm::vec3 Origin = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Direction = glm::vec3(0.0f, 0.0f, 0.0f);
};
