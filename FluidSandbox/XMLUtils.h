#pragma once

#include <vector>
using namespace std;

#include "Utils.h"

#include <rapidxml/rapidxml.hpp>
using namespace rapidxml;

namespace XMLUtils {
	vector<xml_node<>*> getChilds(xml_node<>* parent);
	vector<xml_node<>*> getChilds(xml_node<>* parent, const char* search);
	string findNodeValue(xml_node<>* parent, const char* search, const char* def);
	float findNodeFloat(xml_node<>* parent, const char* search, const float def);
	string getAttribute(xml_node<>* parent, const char* attr, const char* def);
};