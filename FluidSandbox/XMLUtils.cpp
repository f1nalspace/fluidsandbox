/*
======================================================================================================================
	Fluid Sandbox - XMLUtils.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "XMLUtils.h"

XMLUtils::XMLUtils():
	varMng(nullptr) {

}

XMLUtils::XMLUtils(const VariableManager *varMng) :
	varMng(varMng) {
}

std::string XMLUtils::Resolve(const std::string &source) {
	std::string result;
	if(source.length() > 0 && varMng != nullptr) {
		return varMng->Resolve(source);
	}
	return(source);
}

std::vector<const fxmlTag *> XMLUtils::getChilds(const fxmlTag *parent, const char *search) {
	std::vector<const fxmlTag *> r;
	const fxmlTag *cur = parent->firstChild;
	while(cur != nullptr) {
		if(cur->type == fxmlTagType_Element && Utils::isEqual(cur->name, search)) {
			r.push_back(cur);
		}
		cur = cur->nextSibling;
	}
	return r;
}

std::string XMLUtils::getNodeValue(const fxmlTag *parent, const char *search, const char *def) {
	std::string r = def;
	const fxmlTag *foundNode = fxmlFindTagByName(parent, search);
	if(foundNode != nullptr) {
		std::string v = foundNode->value;
		std::string r = Resolve(v);
		return(r);
	}
	return(def);
}

bool XMLUtils::getNodeBool(const fxmlTag *parent, const char *search, const bool def) {
	bool r = def;
	const fxmlTag *foundNode = fxmlFindTagByName(parent, search);
	if(foundNode != nullptr) {
		std::string v = foundNode->value;
		std::string r = Resolve(v);
		bool result = Utils::toBool(r);
		return(result);
	}
	return(def);
}

int32_t XMLUtils::getNodeS32(const fxmlTag *parent, const char *search, const int32_t def) {
	int32_t r = def;
	const fxmlTag *foundNode = fxmlFindTagByName(parent, search);
	if(foundNode != nullptr) {
		std::string v = foundNode->value;
		std::string r = Resolve(v);
		int32_t result = Utils::toS32(r);
		return(result);
	}
	return(def);
}

uint32_t XMLUtils::getNodeU32(const fxmlTag *parent, const char *search, const uint32_t def) {
	uint32_t r = def;
	const fxmlTag *foundNode = fxmlFindTagByName(parent, search);
	if(foundNode != nullptr) {
		std::string v = foundNode->value;
		std::string r = Resolve(v);
		uint32_t result = Utils::toU32(r);
		return(result);
	}
	return(def);
}

float XMLUtils::getNodeFloat(const fxmlTag *parent, const char *search, const float def) {
	float r = def;
	const fxmlTag *foundNode = fxmlFindTagByName(parent, search);
	if(foundNode != nullptr) {
		std::string v = foundNode->value;
		std::string r = Resolve(v);
		float result = Utils::toFloat(r);
		return(result);
	}
	return(def);
}

glm::vec3 XMLUtils::getNodeVec3(const fxmlTag *parent, const char *search, const glm::vec3 &def) {
	glm::vec3 r = def;
	const fxmlTag *foundNode = fxmlFindTagByName(parent, search);
	if(foundNode != nullptr) {
		std::string v = foundNode->value;
		std::string r = Resolve(v);
		glm::vec3 result = Utils::toVec3(r);
		return(result);
	}
	return(def);
}

glm::vec4 XMLUtils::getNodeVec4(const fxmlTag *parent, const char *search, const glm::vec4 &def) {
	const fxmlTag *foundNode = fxmlFindTagByName(parent, search);
	if(foundNode != nullptr) {
		std::string v = foundNode->value;
		std::string r = Resolve(v);
		glm::vec4 result = Utils::toVec4(r);
		return(result);
	}
	return(def);
}

std::string XMLUtils::getAttribute(const fxmlTag *parent, const char *attr, const char *def) {
	const fxmlTag *foundAttr = fxmlFindAttributeByName(parent, attr);
	if(foundAttr != nullptr) {
		std::string v = foundAttr->value;
		std::string r = Resolve(v);
		return(r);
	}
	return def;
}

bool XMLUtils::getAttributeBool(const fxmlTag *parent, const char *attr, const bool def) {
	const fxmlTag *foundAttr = fxmlFindAttributeByName(parent, attr);
	if(foundAttr != nullptr) {
		std::string v = foundAttr->value;
		std::string r = Resolve(v);
		bool result = Utils::toBool(r);
		return(result);
	}
	return def;
}

float XMLUtils::getAttributeFloat(const fxmlTag *parent, const char *attr, const float def) {
	const fxmlTag *foundAttr = fxmlFindAttributeByName(parent, attr);
	if(foundAttr != nullptr) {
		std::string v = foundAttr->value;
		std::string r = Resolve(v);
		float result = Utils::toFloat(r);
		return(result);
	}
	return def;
}

int32_t XMLUtils::getAttributeS32(const fxmlTag *parent, const char *attr, const int32_t def) {
	const fxmlTag *foundAttr = fxmlFindAttributeByName(parent, attr);
	if(foundAttr != nullptr) {
		std::string v = foundAttr->value;
		std::string r = Resolve(v);
		int32_t result = Utils::toS32(r);
		return(result);
	}
	return def;
}

uint32_t XMLUtils::getAttributeU32(const fxmlTag *parent, const char *attr, const uint32_t def) {
	const fxmlTag *foundAttr = fxmlFindAttributeByName(parent, attr);
	if(foundAttr != nullptr) {
		std::string v = foundAttr->value;
		std::string r = Resolve(v);
		uint32_t result = Utils::toU32(r);
		return(result);
	}
	return def;
}

glm::vec3 XMLUtils::getAttributeVec3(const fxmlTag *parent, const char *attr, const glm::vec3 &def) {
	const fxmlTag *foundAttr = fxmlFindAttributeByName(parent, attr);
	if(foundAttr != nullptr) {
		std::string v = foundAttr->value;
		std::string r = Resolve(v);
		glm::vec3 result = Utils::toVec3(r);
		return(result);
	}
	return def;
}

glm::vec4 XMLUtils::getAttributeVec4(const fxmlTag *parent, const char *attr, const glm::vec4 &def) {
	const fxmlTag *foundAttr = fxmlFindAttributeByName(parent, attr);
	if(foundAttr != nullptr) {
		std::string v = foundAttr->value;
		std::string r = Resolve(v);
		glm::vec4 result = Utils::toVec4(r);
		return(result);
	}
	return def;
}