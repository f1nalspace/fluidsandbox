#include "GLSLManager.h"

CGLSLManager::CGLSLManager(void)
{
}

CGLSLManager::~CGLSLManager(void)
{
	for ( map<string, CGLSL*>::const_iterator iter = list.begin();
      iter != list.end(); ++iter ) {
		  CGLSL* glsl = (*iter).second;
		  delete glsl;
		  cout << "    Released shader '" << (*iter).first << "' successfully" << endl;
	}
}

CGLSL* CGLSLManager::add(const string &name, const GLuint what, const string &filename)
{
	CGLSL* shader = get(name);
	if (shader == NULL) {
		shader = new CGLSL();
		list.insert(make_pair(name, shader));
	}
	Utils::attachShaderFromFile(shader, what, filename.c_str(), "  ");
	return shader;
}

CGLSL* CGLSLManager::addVertex(const string &name, const string &filename)
{
	return add(name, GL_VERTEX_SHADER, filename);
}

CGLSL* CGLSLManager::addFragment(const string &name, const string &filename)
{
	return add(name, GL_FRAGMENT_SHADER, filename);
}

CGLSL* CGLSLManager::addGeometry(const string &name, const string &filename)
{
	return add(name, GL_GEOMETRY_SHADER, filename);
}

CGLSL* CGLSLManager::get(const string &name){
	if (list.count(name) > 0)
		return (*list.find(name)).second;
	return NULL;
}