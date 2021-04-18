/*
======================================================================================================================
	Fluid Sandbox - Utils.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

#include "GLSL.h"
#include "OSLowLevel.h"
#include "Actor.hpp"
#include "AllActors.hpp"
#include "ValueTypes.h"

namespace Utils {
	void trim(std::string &str);
	std::vector<std::string> split(const std::string &source, const char *delimiter);
	void replaceString(std::string &value, const std::string &search, const std::string &replace);

	void attachShaderFromFile(CGLSL *shader, const GLuint what, const std::string &filename, const char *indent);
	
	std::vector<char> toCharVector(const std::string &source);
	bool toBool(const std::string &str);
	float toFloat(const std::string &str);
	int32_t toS32(const std::string &str);
	uint32_t toU32(const std::string &str);
	glm::vec3 toVec3(const std::string &str, const glm::vec3 &def = glm::vec3(0));
	glm::vec4 toVec4(const std::string &str, const glm::vec4 &def = glm::vec4(0));
	FluidType toFluidType(const char *str);
	ActorMovementType toActorMovementType(const char *str);

	const std::string toString(const BoolValue &value);
	const std::string toString(const S32Value &value);
	const std::string toString(const FloatValue &value);
	const std::string toString(const Vec3Value &value);
	const std::string toString(const Vec4Value &value);

	enum class EqualityMode {
		CaseSensitive = 0,
		CaseInsensitive
	};

	int compareString(const std::string &a, const std::string &b, const EqualityMode mode = EqualityMode::CaseSensitive);
	bool isEqual(const std::string &a, const std::string &b, const EqualityMode mode = EqualityMode::CaseSensitive);
};