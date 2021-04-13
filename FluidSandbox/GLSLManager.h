#ifndef GLSLMANAGER_H
#define GLSLMANAGER_H

#include <string>
#include <map>
#include <iostream>

#include "GLSL.h"
#include "Utils.h"

class CGLSLManager
{
private:
	std::map<std::string, CGLSL*> nameShaderMap;
	CGLSL* CGLSLManager::add(const std::string &name, const GLuint what, const std::string &filename);
public:
	CGLSLManager(void);
	~CGLSLManager(void);
	CGLSL* addVertex(const std::string &name, const std::string &filename);
	CGLSL* addFragment(const std::string &name, const std::string &filename);
	CGLSL* addGeometry(const std::string &name, const std::string &filename);
	CGLSL* get(const std::string &name);
};
#endif