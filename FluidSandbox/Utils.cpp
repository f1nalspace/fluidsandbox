#include "Utils.h"

namespace Utils
{
	const char* getShaderTypeToString(GLuint what)
	{
		switch (what)
		{
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

	void attachShaderFromFile(CGLSL* shader, GLuint what, const char* filename, const char* indent)
	{
		printf("%sLoad %s shader from file '%s'\n", indent, getShaderTypeToString(what), filename);
		std::string temp = COSLowLevel::getInstance()->getTextFileContent(filename);
		shader->attachShader(what, temp.c_str());
	}

	void replaceString(string& value, string const& search, string const& replace)
	{
		std::string::size_type  next;

		for(next = value.find(search);        // Try and find the first match
			next != std::string::npos;        // next is npos if nothing was found
			next = value.find(search,next)    // search for the next match starting after
											  // the last match that was found.
		   )
		{
			// Inside the loop. So we found a match.
			value.replace(next,search.length(),replace);   // Do the replacement.
			next += replace.length();                      // Move to just after the replace
														   // This is the point were we start
														   // the next search from. 
		}
	}

	vector<char> toCharVector(const string &source)
	{
		vector<char> arr(source.begin(), source.end());
		arr.push_back('\0');
		return arr;
	}

	float toFloat(const string& str)
	{
		stringstream sstr; 
		float f;
		sstr << str;
		sstr >> f; 
		return f;
	}

	int toInt(const string& str)
	{
		stringstream sstr; 
		int i;
		sstr << str;
		sstr >> i; 
		return i;
	}

	unsigned int toUInt(const string& str)
	{
		stringstream sstr; 
		unsigned int i;
		sstr << str;
		sstr >> i; 
		return i;
	}

	string toString(const float value)
	{
		std::ostringstream out;
		out << value;
		return std::string(out.str());
	}

	bool toBool(const string& str)
	{
		if (str.size() == 0) 
			return false;
		else
			return (str.compare("true") == 0) || (str.compare("1") == 0);
	}

	template<typename Operator>
	void tokenize(Operator& op, const char* input, const char* delimiters) {
	  const char* s = input;
	  const char* e = s;
	  while (*e != 0) {
		e = s;
		while (*e != 0 && strchr(delimiters, *e) == 0) ++e;
		if (e - s > 0) {
		  op(s, e - s);
		}
		s = e + 1;
	  }
	}

	class Appender : public std::vector<std::string> {
	public:
	  void operator() (const char* s, unsigned length) { 
		this->push_back(std::string(s,length));
	  }
	};

	vector<string> split(const char* source, const char* delimiter) {
		Appender v;
		tokenize(v, source, delimiter);
		return v;
	}

	void trim(string& str)
	{
	  string::size_type pos = str.find_last_not_of(' ');
	  if(pos != string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != string::npos) str.erase(0, pos);
	  }
	  else str.erase(str.begin(), str.end());
	}

	PxVec3 toVec3(const string& str, const PxVec3 def)
	{
		std::vector<std::string> splitted = split(str.c_str(), ",");
		if (splitted.size() == 3){
			string x = splitted[0];
			string y = splitted[1];
			string z = splitted[2];
			trim(x);
			trim(y);
			trim(z);
			return PxVec3(toFloat(x), toFloat(y), toFloat(z));
		} else { 
			return def;
		}
	}

	PxVec4 toVec4(const string& str)
	{
		std::vector<std::string> splitted = split(str.c_str(), ",");
		if (splitted.size() == 4){
			string x = splitted[0];
			string y = splitted[1];
			string z = splitted[2];
			string w = splitted[3];
			trim(x);
			trim(y);
			trim(z);
			trim(w);
			return PxVec4(toFloat(x), toFloat(y), toFloat(z), toFloat(w));
		} else { 
			return PxVec4(0.0f);
		}
	}

	FluidType toFluidType(const char* str) {
		if (strcmp(str, "sphere") == 0)
			return FluidTypeSphere;
		else if (strcmp(str, "blob") == 0)
			return FluidTypeBlob;
		else if (strcmp(str, "wall") == 0)
			return FluidTypeWall;
		else if (strcmp(str, "drop") == 0)
			return FluidTypeDrop;
		else
			return FluidTypeBlob;
	}

	EActorType toActorType(const char* str) {
		if (strcmp(str, "static") == 0)
			return ActorTypeStatic;
		else if (strcmp(str, "dynamic") == 0)
			return ActorTypeDynamic;
		else
			return ActorTypeStatic;
	}

}
