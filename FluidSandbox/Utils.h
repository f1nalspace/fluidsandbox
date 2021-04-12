#pragma once

#include <string>
#include <vector>

#include <PxPhysicsAPI.h> 

#include "GLSL.h"
#include "OSLowLevel.h"
#include "FluidContainer.hpp"
#include "Actor.h"

namespace Utils {
	void trim(std::string &str);
	std::vector<std::string> split(const std::string &source, const char *delimiter);
	void replaceString(std::string &value, const std::string &search, const std::string &replace);

	void attachShaderFromFile(CGLSL *shader, GLuint what, const std::string &filename, const char *indent);
	
	std::vector<char> toCharVector(const std::string &source);
	float toFloat(const std::string &str);
	int toInt(const std::string &str);
	unsigned int toUInt(const std::string &str);
	bool toBool(const std::string &str);
	PxVec3 toVec3(const std::string &str, const PxVec3 def);
	PxVec4 toVec4(const std::string &str);
	FluidType toFluidType(const char *str);
	EActorType toActorType(const char *str);
	const std::string toString(const float value);
};