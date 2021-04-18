#include "VariableManager.h"

#include <iostream>
#include <queue>

#include <glm/glm.hpp>

#include "Utils.h"
#include "XMLUtils.h"
#include "ValueTypes.h"

static bool IsMathType(const ValueType type) {
	if(type == ValueType::None)
		return(false);
	if(type == ValueType::String)
		return(false);
	if(type == ValueType::Bool)
		return(false);
	return(true);
}
	

static VariableValue ConvertToDefault(const VariableValue &source, const ValueType targetType) {
	switch(targetType) {
		case ValueType::String:
			return VariableValue::fromString("");
		case ValueType::Bool:
			return VariableValue::fromBool(false);
		case ValueType::S32:
			return VariableValue::fromS32(0);
		case ValueType::Float:
			return VariableValue::fromFloat(0);
		case ValueType::Vec3:
			return VariableValue::fromVec3(glm::vec3(0));
		case ValueType::Vec4:
			return VariableValue::fromVec3(glm::vec4(0));
	}
	return(VariableValue(ValueType::None));
}

// String ->
static VariableValue ConvertStringToBool(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::String);
	assert(targetType == ValueType::Bool);
	assert(source.stringValue.value != nullptr);
	return VariableValue::fromBool(Utils::toBool(source.stringValue.value));
}
static VariableValue ConvertStringToS32(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::String);
	assert(targetType == ValueType::S32);
	assert(source.stringValue.value != nullptr);
	return VariableValue::fromS32(Utils::toS32(source.stringValue.value));
}
static VariableValue ConvertStringToFloat(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::String);
	assert(targetType == ValueType::Float);
	assert(source.stringValue.value != nullptr);
	return VariableValue::fromFloat(Utils::toFloat(source.stringValue.value));
}
static VariableValue ConvertStringToVec3(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::String);
	assert(targetType == ValueType::Vec3);
	assert(source.stringValue.value != nullptr);
	return VariableValue::fromVec3(Utils::toVec3(source.stringValue.value));
}
static VariableValue ConvertStringToVec4(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::String);
	assert(targetType == ValueType::Vec4);
	assert(source.stringValue.value != nullptr);
	return VariableValue::fromVec4(Utils::toVec4(source.stringValue.value));
}

// Bool ->
static VariableValue ConvertBoolToString(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Bool);
	assert(targetType == ValueType::String);
	return VariableValue::fromString(Utils::toString(source.boolValue));
}
static VariableValue ConvertBoolToS32(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Bool);
	assert(targetType == ValueType::S32);
	return VariableValue::fromS32(source.boolValue.value ? 1 : 0);
}
static VariableValue ConvertBoolToFloat(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Bool);
	assert(targetType == ValueType::Float);
	return VariableValue::fromFloat(source.boolValue.value ? 1.0f : 0.0f);
}
static VariableValue ConvertBoolToVec3(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Bool);
	assert(targetType == ValueType::Float);
	return VariableValue::fromVec3(glm::vec3(source.boolValue.value ? 1.0f : 0.0f));
}
static VariableValue ConvertBoolToVec4(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Bool);
	assert(targetType == ValueType::Float);
	return VariableValue::fromVec4(glm::vec4(source.boolValue.value ? 1.0f : 0.0f));
}

// S32 ->
static VariableValue ConvertS32ToString(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::S32);
	assert(targetType == ValueType::String);
	return VariableValue::fromString(Utils::toString(source.s32Value));
}
static VariableValue ConvertS32ToBool(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::S32);
	assert(targetType == ValueType::Bool);
	return VariableValue::fromBool(source.s32Value.value ? 1 : 0);
}
static VariableValue ConvertS32ToFloat(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::S32);
	assert(targetType == ValueType::Float);
	return VariableValue::fromFloat((float)source.s32Value.value);
}
static VariableValue ConvertS32ToVec3(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::S32);
	assert(targetType == ValueType::Vec3);
	return VariableValue::fromVec3(glm::vec3((float)source.s32Value.value));
}
static VariableValue ConvertS32ToVec4(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::S32);
	assert(targetType == ValueType::Vec4);
	return VariableValue::fromVec4(glm::vec4((float)source.s32Value.value));
}

// Float ->
static VariableValue ConvertFloatToString(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Float);
	assert(targetType == ValueType::String);
	return VariableValue::fromString(Utils::toString(source.floatValue));
}
static VariableValue ConvertFloatToBool(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Float);
	assert(targetType == ValueType::Bool);
	return VariableValue::fromBool(source.floatValue.value != 0.0f ? true : false);
}
static VariableValue ConvertFloatToS32(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Float);
	assert(targetType == ValueType::S32);
	return VariableValue::fromS32((int32_t)source.floatValue.value);
}
static VariableValue ConvertFloatToVec3(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Float);
	assert(targetType == ValueType::Vec3);
	return VariableValue::fromVec3(glm::vec3(source.floatValue.value));
}
static VariableValue ConvertFloatToVec4(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Float);
	assert(targetType == ValueType::Vec4);
	return VariableValue::fromVec4(glm::vec4(source.floatValue.value));
}

// Vec3 ->
static VariableValue ConvertVec3ToString(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Vec3);
	assert(targetType == ValueType::String);
	return VariableValue::fromString(Utils::toString(source.vec3Value));
}
static VariableValue ConvertVec3ToVec4(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Vec3);
	assert(targetType == ValueType::Vec4);
	return VariableValue::fromVec4(glm::vec4(source.vec3Value.value, 1.0f));
}

// Vec4 ->
static VariableValue ConvertVec4ToString(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Vec4);
	assert(targetType == ValueType::String);
	return VariableValue::fromString(Utils::toString(source.vec4Value));
}
static VariableValue ConvertVec4ToVec3(const VariableValue &source, const ValueType targetType) {
	assert(source.type == ValueType::Vec4);
	assert(targetType == ValueType::Vec3);
	return VariableValue::fromVec3(glm::vec3(source.vec4Value.value));
}

static VariableValue ConvertToNone(const VariableValue &source, const ValueType targetType) {
	return(VariableValue(ValueType::None));
}

static VariableValue ConvertPassthrough(const VariableValue &source, const ValueType targetType) {
	assert(source.type == targetType);
	VariableValue result = VariableValue(source);
	return(result);
}

static const VariableValue ConvertToType(const VariableValue &source, const ValueType targetType) {
	typedef VariableValue ConversionFunc(const VariableValue &source, const ValueType targetType);

	static ConversionFunc *conversionTable[(int)ValueType::Count][(int)ValueType::Count] = {
		nullptr,				// None to None
		ConvertToDefault,		// None to String
		ConvertToDefault,		// None to Bool
		ConvertToDefault,		// None to S32
		ConvertToDefault,		// None to Float
		ConvertToDefault,		// None to Vec3
		ConvertToDefault,		// None to Vec4

		nullptr,				// String to None
		ConvertPassthrough,		// String to String
		ConvertStringToBool,	// String to Bool
		ConvertStringToS32,		// String to S32
		ConvertStringToFloat,	// String to Float
		ConvertStringToVec3,	// String to Vec3
		ConvertStringToVec3,	// String to Vec4

		nullptr,				// Bool to None
		ConvertBoolToString,	// Bool to String
		ConvertPassthrough,		// Bool to Bool
		ConvertBoolToS32,		// Bool to S32
		ConvertBoolToFloat,		// Bool to Float
		ConvertBoolToVec3,		// Bool to Vec3
		ConvertBoolToVec4,		// Bool to Vec4

		nullptr,				// S32 to None
		ConvertS32ToString,		// S32 to String
		ConvertS32ToBool,		// S32 to Bool
		ConvertPassthrough,		// S32 to S32
		ConvertS32ToFloat,		// S32 to Float
		ConvertS32ToVec3,		// S32 to Vec3
		ConvertS32ToVec4,		// S32 to Vec4

		nullptr,				// Float to None
		ConvertFloatToString,	// Float to String
		ConvertFloatToBool,		// Float to Bool
		ConvertFloatToS32,		// Float to S32
		ConvertPassthrough,		// Float to Float
		ConvertFloatToVec3,		// Float to Vec3
		ConvertFloatToVec4,		// Float to Vec4

		nullptr,				// Vec3 to None
		ConvertVec3ToString,	// Vec3 to String
		nullptr,				// Vec3 to Bool
		nullptr,				// Vec3 to S32
		nullptr,				// Vec3 to Float
		ConvertPassthrough,		// Vec3 to Vec3
		ConvertVec3ToVec4,		// Vec3 to Vec4

		nullptr,				// Vec4 to None
		ConvertVec4ToString,	// Vec4 to String
		nullptr,				// Vec4 to Bool
		nullptr,				// Vec4 to S32
		nullptr,				// Vec4 to Float
		ConvertVec4ToVec3,		// Vec4 to Vec3
		ConvertPassthrough,		// Vec4 to Vec4
	};

	ConversionFunc *func = conversionTable[(int)source.type][(int)targetType];
	if(func != nullptr) {
		VariableValue result = func(source, targetType);
		return(result);
	}

	return(VariableValue(ValueType::None));
}

struct Variable {
	enum class Kind: int {
		Fixed = 0,
		Math,
		Reference,
	};

	Kind kind;
	ValueType type;

	Variable(const Kind kind, const ValueType type):
		kind(kind),
		type(type) {
	}
};

struct MathVariable: public Variable {
	enum class Op {
		None = 0,
		Add,
		Sub,
		Div,
		Mult
	};

	Variable *a;
	Variable *b;
	Op op;

	MathVariable(Variable *a, Variable *b, const Op op, const ValueType type):
		Variable(Variable::Kind::Math, type),
		a(a),
		b(b),
		op(op) {
	}

	~MathVariable() {
		if(a != nullptr)
			delete a;
		if(b != nullptr)
			delete b;
	}
};

struct FixedVariable: public Variable {
	VariableValue fixedValue;

	FixedVariable(const VariableValue &fixedValue):
		Variable(Variable::Kind::Fixed, fixedValue.type),
		fixedValue(fixedValue) {
	}
};

struct ReferenceVariable: public Variable {
	std::string referenceName;

	ReferenceVariable(const std::string &referenceName):
		Variable(Variable::Kind::Reference, ValueType::None),
		referenceName(referenceName) {
	}
};

static ValueType ParseType(const std::string &text) {
	if(Utils::isEqual(text, "string", Utils::EqualityMode::CaseInsensitive)) {
		return(ValueType::String);
	} else if(Utils::isEqual(text, "bool", Utils::EqualityMode::CaseInsensitive)) {
		return(ValueType::Bool);
	} else if(Utils::isEqual(text, "int", Utils::EqualityMode::CaseInsensitive) ||
		Utils::isEqual(text, "int32", Utils::EqualityMode::CaseInsensitive) ||
		Utils::isEqual(text, "s32", Utils::EqualityMode::CaseInsensitive)) {
		return(ValueType::S32);
	} else if(Utils::isEqual(text, "float", Utils::EqualityMode::CaseInsensitive) ||
		Utils::isEqual(text, "f32", Utils::EqualityMode::CaseInsensitive)) {
		return(ValueType::Float);
	} else if(Utils::isEqual(text, "vec3", Utils::EqualityMode::CaseInsensitive) ||
		Utils::isEqual(text, "v3", Utils::EqualityMode::CaseInsensitive)) {
		return(ValueType::Vec3);
	} else if(Utils::isEqual(text, "vec4", Utils::EqualityMode::CaseInsensitive) ||
		Utils::isEqual(text, "v4", Utils::EqualityMode::CaseInsensitive)) {
		return(ValueType::Vec4);
	}
	return(ValueType::None);
}

static Variable *ParseVariable(rapidxml::xml_node<> *rootNode) {
	ValueType vtype = ValueType::None;
	auto attr = rootNode->first_attribute();
	while(attr != nullptr) {
		std::string attrName = attr->name();
		if(Utils::isEqual(attrName, "type")) {
			vtype = ParseType(attr->value());
			break;
		}
		attr = attr->next_attribute();
	}

	rapidxml::xml_node<> *first = rootNode->first_node();
	rapidxml::node_type type = first->type();
	if(type == rapidxml::node_type::node_element) {
		std::string nodeName = first->name();
		MathVariable::Op op = MathVariable::Op::None;
		if((Utils::isEqual(nodeName, "Add") && ((op = MathVariable::Op::Add) == MathVariable::Op::Add)) ||
			(Utils::isEqual(nodeName, "Sub") && ((op = MathVariable::Op::Sub) == MathVariable::Op::Sub)) ||
			(Utils::isEqual(nodeName, "Mul") && ((op = MathVariable::Op::Mult) == MathVariable::Op::Mult)) ||
			(Utils::isEqual(nodeName, "Div") && ((op = MathVariable::Op::Div) == MathVariable::Op::Div))) {

			rapidxml::xml_node<> *a = nullptr;
			rapidxml::xml_node<> *b = nullptr;

			rapidxml::xml_node<> *cur = first->first_node();
			while(cur != nullptr) {
				std::string nextName = cur->name();
				if(Utils::isEqual(nextName, "A")) {
					a = cur;
				} else if(Utils::isEqual(nextName, "B")) {
					b = cur;
				} else {
					std::cerr << "Unsupported math operand tag '" << nodeName << "'. Only A and B are allowed!" << std::endl;
				}
				if(a != nullptr && b != nullptr) {
					break;
				}
				cur = cur->next_sibling();
			}
			if(a != nullptr && b != nullptr && op != MathVariable::Op::None) {
				Variable *varA = ParseVariable(a);
				Variable *varB = ParseVariable(b);
				if(vtype == ValueType::None) {
					vtype = ValueType::Float;
				}
				if(varA != nullptr && varB != nullptr && IsMathType(vtype)) {
					MathVariable *result = new MathVariable(varA, varB, op, vtype);
					return(result);
				} else {
					if(varA != nullptr)
						delete varA;
					else if(varB != nullptr)
						delete varB;
				}
			}
		} else if(Utils::isEqual(nodeName, "Var")) {
			std::string varName = first->value();
			ReferenceVariable *result = new ReferenceVariable(varName);
			return(result);
		} else {
			std::cerr << "Unsupported math operation tag '" << nodeName << "'. Only Add/Sub/Mul/Div are allowed!" << std::endl;
		}
	} else if(type == rapidxml::node_data) {
		if(vtype == ValueType::None) {
			vtype = ValueType::String;
		}
		std::string str = rootNode->value();
		VariableValue stringValue = VariableValue::fromString(str);
		VariableValue fixedValue = ConvertToType(stringValue, vtype);
		FixedVariable *result = new FixedVariable(fixedValue);
		return(result);
	}
	return(nullptr);
}

VariableManager::VariableManager() {

}

static const VariableValue ResolveMath(const MathVariable *math, std::map<std::string, VariableValue> &vars);

static const VariableValue ResolveVariable(const Variable *var, std::map<std::string, VariableValue> &vars) {
	VariableValue value = VariableValue(ValueType::None);
	if(var != nullptr) {
		if(var->kind == Variable::Kind::Fixed) {
			const FixedVariable *fixedVar = static_cast<const FixedVariable *>(var);
			value = fixedVar->fixedValue;
		} else if(var->kind == Variable::Kind::Reference) {
			const ReferenceVariable *refVar = static_cast<const ReferenceVariable *>(var);
			if(vars.find(refVar->referenceName) != vars.end()) {
				value = vars[refVar->referenceName];
			}
		} else if(var->kind == Variable::Kind::Math) {
			const MathVariable *mathVar = static_cast<const MathVariable *>(var);
			value = ResolveMath(mathVar, vars);
		}
	}
	return(value);
}



static const VariableValue ResolveMath(const MathVariable *math, std::map<std::string, VariableValue> &vars) {
	assert(math != nullptr);
	assert(math->op != MathVariable::Op::None);
	assert(math->a != nullptr);
	assert(math->b != nullptr);
	const VariableValue a = ResolveVariable(math->a, vars);
	const VariableValue b = ResolveVariable(math->b, vars);
	ValueType vtype = math->type;
	MathVariable::Op op = math->op;

	// Convert A and B to the math type
	VariableValue convA = ConvertToType(a, vtype);
	VariableValue convB = ConvertToType(b, vtype);

	// Compute if both types was converted properly
	if(convA.type == vtype && convB.type == vtype) {
		switch(vtype) {
			case ValueType::S32:
			{
				int32_t value = 0;
				int32_t a = convA.s32Value.value;
				int32_t b = convB.s32Value.value;
				switch(op) {
					case MathVariable::Op::Add:
						value = a + b;
						break;
					case MathVariable::Op::Sub:
						value = a - b;
						break;
					case MathVariable::Op::Mult:
						value = a * b;
						break;
					case MathVariable::Op::Div:
						value = a / b;
						break;
				}
				return VariableValue::fromS32(value);
			} break;

			case ValueType::Float:
			{
				float value = 0;
				float a = convA.floatValue.value;
				float b = convB.floatValue.value;
				switch(op) {
					case MathVariable::Op::Add:
						value = a + b;
						break;
					case MathVariable::Op::Sub:
						value = a - b;
						break;
					case MathVariable::Op::Mult:
						value = a * b;
						break;
					case MathVariable::Op::Div:
						value = a / b;
						break;
				}
				return VariableValue::fromFloat(value);
			} break;

			case ValueType::Vec3:
			{
				glm::vec3 value = glm::vec3(0);
				glm::vec3 a = convA.vec3Value.value;
				glm::vec3 b = convB.vec3Value.value;
				switch(op) {
					case MathVariable::Op::Add:
						value = a + b;
						break;
					case MathVariable::Op::Sub:
						value = a - b;
						break;
					case MathVariable::Op::Mult:
						value = a * b;
						break;
					case MathVariable::Op::Div:
						value = a / b;
						break;
				}
				return VariableValue::fromVec3(value);
			} break;

			case ValueType::Vec4:
			{
				glm::vec4 value = glm::vec4(0);
				glm::vec4 a = convA.vec4Value.value;
				glm::vec4 b = convB.vec4Value.value;
				switch(op) {
					case MathVariable::Op::Add:
						value = a + b;
						break;
					case MathVariable::Op::Sub:
						value = a - b;
						break;
					case MathVariable::Op::Mult:
						value = a * b;
						break;
					case MathVariable::Op::Div:
						value = a / b;
						break;
				}
				return VariableValue::fromVec4(value);
			} break;
		}
	}

	return(VariableValue(ValueType::None));
}


void VariableManager::Parse(rapidxml::xml_node<> *varsNode) {
	vars.clear();

	// Load raw variables tree
	std::map<std::string, Variable *> rawVars;
	auto varNode = varsNode->first_node();
	std::vector<std::pair<std::string, Variable *>> varList;
	while(varNode != nullptr) {
		std::string name = varNode->name();
		Variable *variable = ParseVariable(varNode);
		varList.push_back(std::pair<std::string, Variable *>(name, variable));
		rawVars.insert(std::pair<std::string, Variable *>(name, variable));
		varNode = varNode->next_sibling();
	}

	// Second resolve variables
	for(size_t i = 0, count = varList.size(); i < count; ++i) {
		const std::string &name = varList[i].first;
		Variable *variable = varList[i].second;
		VariableValue value = ResolveVariable(variable, vars);
		vars.insert(std::pair<std::string, VariableValue>(name, value));
	}

	// Remove raw variables tree
	for(auto p = rawVars.begin(); p != rawVars.end(); ++p) {
		Variable *var = p->second;
		if(var != nullptr)
			delete var;
	}
	rawVars.clear();
}

std::string VariableManager::Resolve(const std::string &source) const {
	if(source.length() > 0) {
		std::string result = source;
		for(auto p = vars.begin(); p != vars.end(); ++p) {
			std::string search = p->first;
			search = "{%" + search + "}";
			VariableValue value = p->second;
			std::string textValue = value.toString();
			Utils::replaceString(result, search, textValue);
		}
		return(result);
	}
	return(source);
}

VariableValue::VariableValue(const ValueType type):
	type(type),
	vec4Value(Vec4Value()),
	vec3Value(Vec3Value()),
	stringValue(StringValue()),
	floatValue(FloatValue()),
	s32Value(S32Value()),
	boolValue(BoolValue()) {
}

VariableValue::VariableValue(): VariableValue(ValueType::None) {}

VariableValue &VariableValue::operator=(const VariableValue &other) {
	type = other.type;
	boolValue = other.boolValue;
	s32Value = other.s32Value;
	floatValue = other.floatValue;
	vec3Value = other.vec3Value;
	vec4Value = other.vec4Value;
	stringValue = other.stringValue;
	return(*this);
}

VariableValue::~VariableValue() {
}

VariableValue::VariableValue(const VariableValue &other):
	type(other.type),
	vec4Value(other.vec4Value),
	vec3Value(other.vec3Value),
	stringValue(other.stringValue),
	floatValue(other.floatValue),
	s32Value(other.s32Value),
	boolValue(other.boolValue) {
}

VariableValue VariableValue::fromS32(const int32_t value) {
	VariableValue result = VariableValue(ValueType::S32);
	result.s32Value.value = value;
	return(result);
}
VariableValue VariableValue::fromFloat(const float value) {
	VariableValue result = VariableValue(ValueType::Float);
	result.floatValue.value = value;
	return(result);
}
VariableValue VariableValue::fromString(const std::string &value) {
	VariableValue result = VariableValue(ValueType::String);
	result.stringValue = StringValue::fromString(value);
	return(result);
}
VariableValue VariableValue::fromBool(const bool value) {
	VariableValue result = VariableValue(ValueType::Bool);
	result.boolValue.value = value;
	return(result);
}
VariableValue VariableValue::fromVec3(const glm::vec3 &value) {
	VariableValue result = VariableValue(ValueType::Vec3);
	result.vec3Value.value = value;
	return(result);
}
VariableValue VariableValue::fromVec4(const glm::vec4 &value) {
	VariableValue result = VariableValue(ValueType::Vec4);
	result.vec4Value.value = value;
	return(result);
}

std::string VariableValue::toString() const {
	switch(type) {
		case ValueType::String:
			return std::string(stringValue.value);
		case ValueType::Bool:
			return Utils::toString(boolValue);
		case ValueType::S32:
			return Utils::toString(s32Value);
		case ValueType::Float:
			return Utils::toString(floatValue);
		case ValueType::Vec3:
			return Utils::toString(vec3Value);
		case ValueType::Vec4:
			return Utils::toString(vec4Value);
		default:
			return("");
	}
}
