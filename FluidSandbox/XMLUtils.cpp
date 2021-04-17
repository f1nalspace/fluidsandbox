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

std::vector<rapidxml::xml_node<> *> XMLUtils::getChilds(rapidxml::xml_node<> *parent) {
	std::vector<rapidxml::xml_node<> *> r;
	rapidxml::xml_node<> *cur = parent->first_node();
	if(cur) {
		r.push_back(cur);
		while((cur = cur->next_sibling())) {
			r.push_back(cur);
		}
	}
	return r;
}

std::vector<rapidxml::xml_node<> *> XMLUtils::getChilds(rapidxml::xml_node<> *parent, const char *search) {
	std::vector<rapidxml::xml_node<> *> r;
	rapidxml::xml_node<> *cur = parent->first_node(search);
	if(cur) {
		r.push_back(cur);
		while((cur = cur->next_sibling(search))) {
			r.push_back(cur);
		}
	}
	return r;
}

std::string XMLUtils::getNodeValue(rapidxml::xml_node<> *parent, const char *search, const char *def) {
	std::string r = def;
	rapidxml::xml_node<> *foundNode = parent->first_node(search);
	if(foundNode) {
		std::string v = foundNode->value();
		std::string r = Resolve(v);
		return(r);
	}
	return(def);
}

bool XMLUtils::getNodeBool(rapidxml::xml_node<> *parent, const char *search, const bool def) {
	bool r = def;
	rapidxml::xml_node<> *foundNode = parent->first_node(search);
	if(foundNode) {
		std::string v = foundNode->value();
		std::string r = Resolve(v);
		bool result = Utils::toBool(r);
		return(result);
	}
	return(def);
}

int32_t XMLUtils::getNodeS32(rapidxml::xml_node<> *parent, const char *search, const int32_t def) {
	int32_t r = def;
	rapidxml::xml_node<> *foundNode = parent->first_node(search);
	if(foundNode) {
		std::string v = foundNode->value();
		std::string r = Resolve(v);
		int32_t result = Utils::toS32(r);
		return(result);
	}
	return(def);
}

uint32_t XMLUtils::getNodeU32(rapidxml::xml_node<> *parent, const char *search, const uint32_t def) {
	uint32_t r = def;
	rapidxml::xml_node<> *foundNode = parent->first_node(search);
	if(foundNode) {
		std::string v = foundNode->value();
		std::string r = Resolve(v);
		uint32_t result = Utils::toU32(r);
		return(result);
	}
	return(def);
}

float XMLUtils::getNodeFloat(rapidxml::xml_node<> *parent, const char *search, const float def) {
	float r = def;
	rapidxml::xml_node<> *foundNode = parent->first_node(search);
	if(foundNode) {
		std::string v = foundNode->value();
		std::string r = Resolve(v);
		float result = Utils::toFloat(r);
		return(result);
	}
	return(def);
}

glm::vec3 XMLUtils::getNodeVec3(rapidxml::xml_node<> *parent, const char *search, const glm::vec3 &def) {
	glm::vec3 r = def;
	rapidxml::xml_node<> *foundNode = parent->first_node(search);
	if(foundNode) {
		std::string v = foundNode->value();
		std::string r = Resolve(v);
		glm::vec3 result = Utils::toVec3(r);
		return(result);
	}
	return(def);
}

glm::vec4 XMLUtils::getNodeVec4(rapidxml::xml_node<> *parent, const char *search, const glm::vec4 &def) {
	rapidxml::xml_node<> *foundNode = parent->first_node(search);
	if(foundNode) {
		std::string v = foundNode->value();
		std::string r = Resolve(v);
		glm::vec4 result = Utils::toVec4(r);
		return(result);
	}
	return(def);
}

std::string XMLUtils::getAttribute(rapidxml::xml_node<> *parent, const char *attr, const char *def) {
	rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
	if(foundAttr) {
		std::string v = foundAttr->value();
		std::string r = Resolve(v);
		return(r);
	}
	return def;
}

bool XMLUtils::getAttributeBool(rapidxml::xml_node<> *parent, const char *attr, const bool def) {
	rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
	if(foundAttr) {
		std::string v = foundAttr->value();
		std::string r = Resolve(v);
		bool result = Utils::toBool(r);
		return(result);
	}
	return def;
}

float XMLUtils::getAttributeFloat(rapidxml::xml_node<> *parent, const char *attr, const float def) {
	rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
	if(foundAttr) {
		std::string v = foundAttr->value();
		std::string r = Resolve(v);
		float result = Utils::toFloat(r);
		return(result);
	}
	return def;
}

int32_t XMLUtils::getAttributeS32(rapidxml::xml_node<> *parent, const char *attr, const int32_t def) {
	rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
	if(foundAttr) {
		std::string v = foundAttr->value();
		std::string r = Resolve(v);
		int32_t result = Utils::toS32(r);
		return(result);
	}
	return def;
}

uint32_t XMLUtils::getAttributeU32(rapidxml::xml_node<> *parent, const char *attr, const uint32_t def) {
	rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
	if(foundAttr) {
		std::string v = foundAttr->value();
		std::string r = Resolve(v);
		uint32_t result = Utils::toU32(r);
		return(result);
	}
	return def;
}

glm::vec3 XMLUtils::getAttributeVec3(rapidxml::xml_node<> *parent, const char *attr, const glm::vec3 &def) {
	rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
	if(foundAttr) {
		std::string v = foundAttr->value();
		std::string r = Resolve(v);
		glm::vec3 result = Utils::toVec3(r);
		return(result);
	}
	return def;
}

glm::vec4 XMLUtils::getAttributeVec4(rapidxml::xml_node<> *parent, const char *attr, const glm::vec4 &def) {
	rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
	if(foundAttr) {
		std::string v = foundAttr->value();
		std::string r = Resolve(v);
		glm::vec4 result = Utils::toVec4(r);
		return(result);
	}
	return def;
}