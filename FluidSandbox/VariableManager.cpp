#include "VariableManager.h"

#include <iostream>
#include <queue>

#include "Utils.h"
#include "XMLUtils.h"

struct Variable {
	enum class Kind: int {
		Fixed = 0,
		Math,
		Reference,
	};

	Kind kind;

	Variable(const Kind kind): kind(kind) {

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

	MathVariable(Variable *a, Variable *b, const Op op):
		Variable(Variable::Kind::Math),
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
	std::string fixedValue;

	FixedVariable(const std::string &fixedValue):
		Variable(Variable::Kind::Fixed),
		fixedValue(fixedValue) {
	}
};

struct ReferenceVariable: public Variable {
	std::string referenceName;

	ReferenceVariable(const std::string &referenceName):
		Variable(Variable::Kind::Reference),
		referenceName(referenceName) {
	}
};

static Variable *parseVariable(rapidxml::xml_node<> *rootNode) {
	rapidxml::xml_node<> *first = rootNode->first_node();
	rapidxml::node_type type = first->type();
	if(type == rapidxml::node_type::node_element) {
		std::string nodeName = first->name();
		MathVariable::Op op = MathVariable::Op::None;
		if((nodeName.compare("Add") == 0 && ((op = MathVariable::Op::Add) == MathVariable::Op::Add)) ||
			(nodeName.compare("Sub") == 0 && ((op = MathVariable::Op::Sub) == MathVariable::Op::Sub)) ||
			(nodeName.compare("Mul") == 0 && ((op = MathVariable::Op::Mult) == MathVariable::Op::Mult)) ||
			(nodeName.compare("Div") == 0 && ((op = MathVariable::Op::Div) == MathVariable::Op::Div))) {

			rapidxml::xml_node<> *a = nullptr;
			rapidxml::xml_node<> *b = nullptr;

			rapidxml::xml_node<> *cur = first->first_node();
			while(cur != nullptr) {
				std::string nextName = cur->name();
				if(nextName.compare("A") == 0) {
					a = cur;
				} else if(nextName.compare("B") == 0) {
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
				Variable *varA = parseVariable(a);
				Variable *varB = parseVariable(b);
				if(varA != nullptr && varB != nullptr) {
					MathVariable *result = new MathVariable(varA, varB, op);
					return(result);
				} else {
					if(varA != nullptr)
						delete varA;
					else if(varB != nullptr)
						delete varB;
				}
			}
		} else if(nodeName.compare("Var") == 0) {
			std::string varName = first->value();
			ReferenceVariable *result = new ReferenceVariable(varName);
			return(result);
		} else {
			std::cerr << "Unsupported math operation tag '" << nodeName << "'. Only Add/Sub/Mul/Div are allowed!" << std::endl;
		}
	} else if(type == rapidxml::node_data) {
		FixedVariable *result = new FixedVariable(rootNode->value());
		return(result);
	}
	return(nullptr);
}

VariableManager::VariableManager() {

}

void VariableManager::Parse(rapidxml::xml_node<> *varsNode) {
	vars.clear();

	// Load raw variables tree
	std::map<std::string, Variable *> rawVars;
	auto varNode = varsNode->first_node();
	std::vector<std::pair<std::string, Variable *>> varList;
	while(varNode != nullptr) {
		std::string name = varNode->name();
		Variable *variable = parseVariable(varNode);
		varList.push_back(std::pair<std::string, Variable *>(name, variable));
		rawVars.insert(std::pair<std::string, Variable *>(name, variable));
		varNode = varNode->next_sibling();
	}

	// First resolve all fixed values
	for(size_t i = 0, count = varList.size(); i < count; ++i) {
		const std::string &name = varList[i].first;
		Variable *variable = varList[i].second;
		if(variable->kind == Variable::Kind::Fixed) {
			FixedVariable *fixedVar = static_cast<FixedVariable *>(variable);
			vars.insert(std::pair<std::string, std::string>(name, fixedVar->fixedValue));
		}
	}

	// Second resolve all math and reference variables
	for(size_t i = 0, count = varList.size(); i < count; ++i) {
		const std::string &name = varList[i].first;
		Variable *variable = varList[i].second;
		if(variable->kind == Variable::Kind::Math) {
			MathVariable *mathVar = static_cast<MathVariable *>(variable);
			//vars.insert(std::pair<std::string, std::string>(name, fixedVar->fixedValue));
		} else if(variable->kind == Variable::Kind::Reference) {
			ReferenceVariable *refVar = static_cast<ReferenceVariable *>(variable);
			//vars.insert(std::pair<std::string, std::string>(name, fixedVar->fixedValue));
		}
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
			std::string value = p->second;
			Utils::replaceString(result, search, value);
		}
		return(result);
	}
	return(source);
}
