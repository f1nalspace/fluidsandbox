/*
======================================================================================================================
	Fluid Sandbox - Utils.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "Utils.h"

#include <sstream>

namespace Utils {
	const char *getShaderTypeToString(const GLuint what) {
		switch(what) {
			case GL_VERTEX_SHADER:
				return "Vertex";
			case GL_FRAGMENT_SHADER:
				return "Fragment";
			case GL_GEOMETRY_SHADER:
				return "Geometry";
			default:
				return "Unknown";
		}
	}

	void replaceString(std::string &value, const std::string &search, const std::string &replace) {
		std::string::size_type  next;

		for(next = value.find(search);        // Try and find the first match
			next != std::string::npos;        // next is npos if nothing was found
			next = value.find(search, next)    // search for the next match starting after
											  // the last match that was found.
			) {
			 // Inside the loop. So we found a match.
			value.replace(next, search.length(), replace);   // Do the replacement.
			next += replace.length();                        // Move to just after the replace
														     // This is the point were we start
														     // the next search from. 
		}
	}

	std::vector<std::string> split(const std::string &source, const char *delimiter) {
		std::vector<std::string> result;
		const char *s = source.c_str();
		const char *e = s;
		while(*e != 0) {
			e = s;
			while(*e != 0 && strchr(delimiter, *e) == 0) ++e;
			if(e - s > 0) {
				size_t len = e - s;
				result.push_back(std::string(s, len));
			}
			s = e + 1;
		}
		return(result);
	}

	void trim(std::string &str) {
		std::string::size_type pos = str.find_last_not_of(' ');
		if(pos != std::string::npos) {
			str.erase(pos + 1);
			pos = str.find_first_not_of(' ');
			if(pos != std::string::npos) str.erase(0, pos);
		} else str.erase(str.begin(), str.end());
	}

	void attachShaderFromFile(CGLSL *shader, const GLuint what, const std::string &filename, const char *indent) {
		const char *whatName = getShaderTypeToString(what);
		printf("%sLoad %s shader from file '%s'\n", indent, whatName, filename.c_str());
		std::string temp = COSLowLevel::getTextFileContent(filename);
		shader->attachShader(what, temp.c_str());
	}

	std::vector<char> toCharVector(const std::string &source) {
		std::vector<char> arr(source.begin(), source.end());
		arr.push_back('\0');
		return arr;
	}

	int compareString(const std::string &a, const std::string &b, const EqualityMode mode) {
		int r;
		if(mode == EqualityMode::CaseSensitive) {
			r = a.compare(b);
		} else if(mode == EqualityMode::CaseInsensitive) {
			size_t sizeA = a.size();
			size_t sizeB = b.size();
			if(sizeA < sizeB) {
				r = -1;
			} else if(sizeA > sizeB) {
				r = 1;
			} else {
				r = 0;
				for(size_t i = 0; i < sizeA; ++i) {
					if(tolower(a[i]) != tolower(b[i])) {
						r = 1;
						break;
					}
				}
			}
		}
		return(r);
	}

	bool isEqual(const std::string &a, const std::string &b, const EqualityMode mode) {
		int r = compareString(a, b, mode);
		bool result = r == 0;
		return(result);
	}

	float toFloat(const std::string &str) {
		std::stringstream sstr;
		float f;
		sstr << str;
		sstr >> f;
		return f;
	}

	int32_t toS32(const std::string &str) {
		std::stringstream sstr;
		int32_t i;
		sstr << str;
		sstr >> i;
		return i;
	}

	uint32_t toU32(const std::string &str) {
		std::stringstream sstr;
		uint32_t i;
		sstr << str;
		sstr >> i;
		return i;
	}

	const std::string toString(const BoolValue &value) {
		if(value.value)
			return "true";
		else
			return "false";
	}
	const std::string toString(const FloatValue &value) {
		std::ostringstream out;
		out << value.value;
		return std::string(out.str());
	}
	const std::string toString(const S32Value &value) {
		std::ostringstream out;
		out << value.value;
		return std::string(out.str());
	}
	const std::string toString(const Vec3Value &value) {
		std::ostringstream out;
		out << value.value.x << "," << value.value.y << "," << value.value.z;
		return std::string(out.str());
	}
	const std::string toString(const Vec4Value &value) {
		std::ostringstream out;
		out << value.value.x << "," << value.value.y << "," << value.value.z << "," << value.value.w;
		return std::string(out.str());
	}

	bool toBool(const std::string &str) {
		if(str.size() == 0)
			return false;
		else
			return (str.compare("true") == 0) || (str.compare("1") == 0);
	}

	glm::vec3 toVec3(const std::string &str, const glm::vec3 &def) {
		std::vector<std::string> splitted = split(str, ",");
		if(splitted.size() == 3) {
			std::string x = splitted[0];
			std::string y = splitted[1];
			std::string z = splitted[2];
			trim(x);
			trim(y);
			trim(z);
			return glm::vec3(toFloat(x), toFloat(y), toFloat(z));
		} else {
			return def;
		}
	}

	glm::vec4 toVec4(const std::string &str, const glm::vec4 &def) {
		std::vector<std::string> splitted = split(str.c_str(), ",");
		if(splitted.size() == 4) {
			std::string x = splitted[0];
			std::string y = splitted[1];
			std::string z = splitted[2];
			std::string w = splitted[3];
			trim(x);
			trim(y);
			trim(z);
			trim(w);
			return glm::vec4(toFloat(x), toFloat(y), toFloat(z), toFloat(w));
		} else {
			return def;
		}
	}

	FluidType toFluidType(const char *str) {
		if(strcmp(str, "sphere") == 0)
			return FluidType::Sphere;
		else if(strcmp(str, "blob") == 0 || strcmp(str, "box") == 0 || strcmp(str, "cube") == 0)
			return FluidType::Box;
		else if(strcmp(str, "wall") == 0 || strcmp(str, "plane") == 0)
			return FluidType::Plane;
		else if(strcmp(str, "drop"))
			return FluidType::Drop;
		else
			return FluidType::Box;
	}

	ActorMovementType toActorMovementType(const char *str) {
		if(strcmp(str, "static") == 0)
			return ActorMovementType::Static;
		else if(strcmp(str, "dynamic") == 0)
			return ActorMovementType::Dynamic;
		else
			return ActorMovementType::Static;
	}
}
