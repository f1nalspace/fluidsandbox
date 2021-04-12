#include "XMLUtils.h"

namespace XMLUtils
{
	std::vector<rapidxml::xml_node<>*> getChilds(rapidxml::xml_node<>* parent) {
		std::vector<rapidxml::xml_node<>*> r;
		rapidxml::xml_node<> *cur = parent->first_node();
		if (cur) {
			r.push_back(cur);
			while ((cur = cur->next_sibling())) {
				r.push_back(cur);
			}
		}
		return r;
	}

	std::vector<rapidxml::xml_node<>*> getChilds(rapidxml::xml_node<>* parent, const char* search)
	{
		std::vector<rapidxml::xml_node<>*> r;
		rapidxml::xml_node<> *cur = parent->first_node(search);
		if (cur) {
			r.push_back(cur);
			while ((cur = cur->next_sibling(search))) {
				r.push_back(cur);
			}
		}
		return r;
	}

	std::string findNodeValue(rapidxml::xml_node<>* parent, const char* search, const char* def) {
		std::string r = def;
		rapidxml::xml_node<> *foundNode = parent->first_node(search);
		if (foundNode) {
			r = foundNode->value();
		}
		return r;
	}

	float findNodeFloat(rapidxml::xml_node<>* parent, const char* search, const float def)
	{
		float r = def;
		rapidxml::xml_node<> *foundNode = parent->first_node(search);
		if (foundNode) {
			r = Utils::toFloat(foundNode->value());
		}
		return r;
	}

	std::string getAttribute(rapidxml::xml_node<>* parent, const char* attr, const char* def)
	{
		rapidxml::xml_attribute<> *foundAttr = parent->first_attribute(attr);
		if (foundAttr) {
			return foundAttr->value();
		} else {
			return def;
		}
	}
}