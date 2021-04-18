#pragma once

#include <cstdint>
#include <string>

#include <glm/glm.hpp>

enum class ValueType: int {
	None = 0,
	String,
	Bool,
	S32,
	Float,
	Vec3,
	Vec4,
	Count
};

struct BoolValue {
	bool value;
};

struct FloatValue {
	float value;
};

struct S32Value {
	int32_t value;
};

struct Vec3Value {
	glm::vec3 value;
};

struct Vec4Value {
	glm::vec4 value;
};

struct StringValue {
	char value[255];

	static StringValue fromString(const std::string &source) {
		StringValue result = {};
		strcpy_s(result.value, sizeof(result.value), source.c_str());
		return(result);
	}
};