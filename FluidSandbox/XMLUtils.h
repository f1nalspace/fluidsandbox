/*
======================================================================================================================
	Fluid Sandbox - XMLUtils.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

#include <final_xml.h>

#include "Utils.h"
#include "VariableManager.h"

struct XMLUtils {
private:
	const VariableManager *varMng;
	std::string Resolve(const std::string &source);
public:
	XMLUtils();
	XMLUtils(const VariableManager *varMng);

	std::vector<const fxmlTag *> getChilds(const fxmlTag *parent, const char *search);

	std::string getNodeValue(const fxmlTag *parent, const char *search, const char *def);
	bool getNodeBool(const fxmlTag *parent, const char *search, const bool def);
	int32_t getNodeS32(const fxmlTag *parent, const char *search, const int32_t def);
	uint32_t getNodeU32(const fxmlTag *parent, const char *search, const uint32_t def);
	float getNodeFloat(const fxmlTag *parent, const char *search, const float def);
	glm::vec3 getNodeVec3(const fxmlTag *parent, const char *search, const glm::vec3 &def);
	glm::vec4 getNodeVec4(const fxmlTag *parent, const char *search, const glm::vec4 &def);

	std::string getAttribute(const fxmlTag *parent, const char *attr, const char *def);
	bool getAttributeBool(const fxmlTag *parent, const char *attr, const bool def);
	int32_t getAttributeS32(const fxmlTag *parent, const char *attr, const int32_t def);
	uint32_t getAttributeU32(const fxmlTag *parent, const char *attr, const uint32_t def);
	float getAttributeFloat(const fxmlTag *parent, const char *attr, const float def);
	glm::vec3 getAttributeVec3(const fxmlTag *parent, const char *attr, const glm::vec3 &def);
	glm::vec4 getAttributeVec4(const fxmlTag *parent, const char *attr, const glm::vec4 &def);
};