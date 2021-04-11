#pragma once

#include <string>
#include <vector>
#include <sstream>
using namespace std;

#include <PxPhysicsAPI.h> 

#include "GLSL.h"
#include "OSLowLevel.h"
#include "FluidContainer.hpp"
#include "Actor.h"

namespace Utils
{
	void attachShaderFromFile(CGLSL* shader, GLuint what, const char* filename, const char* indent);
	void replaceString(string& value, string const& search, string const& replace);
	vector<char> toCharVector(const string& source);
	float toFloat(const string& str);
	int toInt(const string& str);
	unsigned int toUInt(const string& str);
	bool toBool(const string& str);
	PxVec3 toVec3(const string& str, const PxVec3 def);
	PxVec4 toVec4(const string& str);
	FluidType toFluidType(const char* str);
	EActorType toActorType(const char* str);
	string toString(const float value);
};