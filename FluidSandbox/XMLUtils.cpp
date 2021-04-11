#include "XMLUtils.h"

namespace XMLUtils
{
	vector<xml_node<>*> getChilds(xml_node<>* parent) {
		vector<xml_node<>*> r;
		xml_node<> *cur = parent->first_node();
		if (cur) {
			r.push_back(cur);
			while ((cur = cur->next_sibling())) {
				r.push_back(cur);
			}
		}
		return r;
	}

	vector<xml_node<>*> getChilds(xml_node<>* parent, const char* search)
	{
		vector<xml_node<>*> r;
		xml_node<> *cur = parent->first_node(search);
		if (cur) {
			r.push_back(cur);
			while ((cur = cur->next_sibling(search))) {
				r.push_back(cur);
			}
		}
		return r;
	}

	string findNodeValue(xml_node<>* parent, const char* search, const char* def) {
		string r = def;
		xml_node<> *foundNode = parent->first_node(search);
		if (foundNode) {
			r = foundNode->value();
		}
		return r;
	}

	float findNodeFloat(xml_node<>* parent, const char* search, const float def)
	{
		float r = def;
		xml_node<> *foundNode = parent->first_node(search);
		if (foundNode) {
			r = Utils::toFloat(foundNode->value());
		}
		return r;
	}

	string getAttribute(xml_node<>* parent, const char* attr, const char* def)
	{
		xml_attribute<> *foundAttr = parent->first_attribute(attr);
		if (foundAttr) {
			return foundAttr->value();
		} else {
			return def;
		}
	}
}