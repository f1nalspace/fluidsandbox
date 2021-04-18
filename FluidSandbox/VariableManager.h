#pragma once

#include <map>
#include <string>

#include <rapidxml/rapidxml.hpp>

#include "ValueTypes.h"

struct VariableValue {
	ValueType type;

	union {
		Vec4Value vec4Value;
		Vec3Value vec3Value;
		StringValue stringValue;
		FloatValue floatValue;
		S32Value s32Value;
		BoolValue boolValue;
	};

	VariableValue(const ValueType type);
	VariableValue();
	VariableValue &operator=(const VariableValue &other);

	VariableValue(const VariableValue &other);
	~VariableValue();

	static VariableValue fromS32(const int32_t value);
	static VariableValue fromFloat(const float value);
	static VariableValue fromString(const std::string &value);
	static VariableValue fromBool(const bool value);
	static VariableValue fromVec3(const glm::vec3 &value);
	static VariableValue fromVec4(const glm::vec4 &value);

	std::string toString() const;
};

struct VariableManager {
private:
	
public:
	std::map<std::string, VariableValue> vars;
	VariableManager();
	void Parse(rapidxml::xml_node<> *varsNode);
	std::string Resolve(const std::string &source) const;
};

