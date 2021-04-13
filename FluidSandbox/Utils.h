#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "GLSL.h"
#include "OSLowLevel.h"
#include "FluidContainer.hpp"
#include "Actor.hpp"

namespace Utils {
	void trim(std::string &str);
	std::vector<std::string> split(const std::string &source, const char *delimiter);
	void replaceString(std::string &value, const std::string &search, const std::string &replace);

	void attachShaderFromFile(CGLSL *shader, const GLuint what, const std::string &filename, const char *indent);
	
	std::vector<char> toCharVector(const std::string &source);
	float toFloat(const std::string &str);
	int toInt(const std::string &str);
	unsigned int toUInt(const std::string &str);
	bool toBool(const std::string &str);
	glm::vec3 toVec3(const std::string &str, const glm::vec3 &def = glm::vec3(0));
	glm::vec4 toVec4(const std::string &str, const glm::vec4 &def = glm::vec4(0));
	FluidType toFluidType(const char *str);
	ActorMovementType toActorMovementType(const char *str);
	const std::string toString(const float value);
};