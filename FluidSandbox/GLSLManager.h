#ifndef GLSLMANAGER_H
#define GLSLMANAGER_H
#include <string>
#include <map>
#include <iostream>
using namespace std;
#include "GLSL.h"
#include "Utils.h"
class CGLSLManager
{
private:
	map<string, CGLSL*> list;
	CGLSL* CGLSLManager::add(const string &name, const GLuint what, const string &filename);
public:
	CGLSLManager(void);
	~CGLSLManager(void);
	CGLSL* addVertex(const string &name, const string &filename);
	CGLSL* addFragment(const string &name, const string &filename);
	CGLSL* addGeometry(const string &name, const string &filename);
	CGLSL* get(const string &name);
};
#endif