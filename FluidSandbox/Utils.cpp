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

	void attachShaderFromFile(CGLSL *shader, GLuint what, const std::string &filename, const char *indent) {
		printf("%sLoad %s shader from file '%s'\n", indent, getShaderTypeToString(what), filename);
		std::string temp = COSLowLevel::getInstance()->getTextFileContent(filename);
		shader->attachShader(what, temp.c_str());
	}

	std::vector<char> toCharVector(const std::string &source) {
		std::vector<char> arr(source.begin(), source.end());
		arr.push_back('\0');
		return arr;
	}

	float toFloat(const std::string &str) {
		std::stringstream sstr;
		float f;
		sstr << str;
		sstr >> f;
		return f;
	}

	int toInt(const std::string &str) {
		std::stringstream sstr;
		int i;
		sstr << str;
		sstr >> i;
		return i;
	}

	unsigned int toUInt(const std::string &str) {
		std::stringstream sstr;
		unsigned int i;
		sstr << str;
		sstr >> i;
		return i;
	}

	const std::string toString(const float value) {
		std::ostringstream out;
		out << value;
		return std::string(out.str());
	}

	bool toBool(const std::string &str) {
		if(str.size() == 0)
			return false;
		else
			return (str.compare("true") == 0) || (str.compare("1") == 0);
	}

	PxVec3 toVec3(const std::string &str, const PxVec3 def) {
		std::vector<std::string> splitted = split(str, ",");
		if(splitted.size() == 3) {
			std::string x = splitted[0];
			std::string y = splitted[1];
			std::string z = splitted[2];
			trim(x);
			trim(y);
			trim(z);
			return PxVec3(toFloat(x), toFloat(y), toFloat(z));
		} else {
			return def;
		}
	}

	PxVec4 toVec4(const std::string &str) {
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
			return PxVec4(toFloat(x), toFloat(y), toFloat(z), toFloat(w));
		} else {
			return PxVec4(0.0f);
		}
	}

	FluidType toFluidType(const char *str) {
		if(strcmp(str, "sphere") == 0)
			return FluidTypeSphere;
		else if(strcmp(str, "blob") == 0)
			return FluidTypeBlob;
		else if(strcmp(str, "wall") == 0)
			return FluidTypeWall;
		else if(strcmp(str, "drop") == 0)
			return FluidTypeDrop;
		else
			return FluidTypeBlob;
	}

	EActorType toActorType(const char *str) {
		if(strcmp(str, "static") == 0)
			return ActorTypeStatic;
		else if(strcmp(str, "dynamic") == 0)
			return ActorTypeDynamic;
		else
			return ActorTypeStatic;
	}
}