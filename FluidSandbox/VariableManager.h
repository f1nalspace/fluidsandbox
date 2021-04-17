#pragma once

#include <map>
#include <string>

#include <rapidxml/rapidxml.hpp>

struct VariableManager {
	std::map<std::string, std::string> vars;
	VariableManager();
	void Parse(rapidxml::xml_node<> *varsNode);
	std::string Resolve(const std::string &source) const;
};

