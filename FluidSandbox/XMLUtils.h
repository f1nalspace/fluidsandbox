/*
======================================================================================================================
	Fluid Sandbox - XMLUtils.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <vector>

#include "Utils.h"

#include <rapidxml/rapidxml.hpp>

namespace XMLUtils {
	std::vector<rapidxml::xml_node<>*> getChilds(rapidxml::xml_node<>* parent);
	std::vector<rapidxml::xml_node<>*> getChilds(rapidxml::xml_node<>* parent, const char* search);
	std::string findNodeValue(rapidxml::xml_node<>* parent, const char* search, const char* def);
	float findNodeFloat(rapidxml::xml_node<>* parent, const char* search, const float def);
	std::string getAttribute(rapidxml::xml_node<>* parent, const char* attr, const char* def);
};