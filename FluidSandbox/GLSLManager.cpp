#include "GLSLManager.h"

CGLSLManager::CGLSLManager(void)
{
}

CGLSLManager::~CGLSLManager(void)
{
	for (std::map<std::string, CGLSL*>::const_iterator iter = list.begin();
      iter != list.end(); ++iter ) {
		  CGLSL* glsl = (*iter).second;
		  delete glsl;
		  std::cout << "    Released shader '" << (*iter).first << "' successfully" << std::endl;
	}
}

CGLSL* CGLSLManager::add(const std::string &name, const GLuint what, const std::string &filename)
{
	CGLSL* shader = get(name);
	if (shader == NULL) {
		shader = new CGLSL();
		list.insert(std::make_pair(name, shader));
	}
	Utils::attachShaderFromFile(shader, what, filename.c_str(), "  ");
	return shader;
}

CGLSL* CGLSLManager::addVertex(const std::string &name, const std::string &filename)
{
	return add(name, GL_VERTEX_SHADER, filename);
}

CGLSL* CGLSLManager::addFragment(const std::string &name, const std::string &filename)
{
	return add(name, GL_FRAGMENT_SHADER, filename);
}

CGLSL* CGLSLManager::addGeometry(const std::string &name, const std::string &filename)
{
	return add(name, GL_GEOMETRY_SHADER, filename);
}

CGLSL* CGLSLManager::get(const std::string &name){
	if (list.count(name) > 0)
		return (*list.find(name)).second;
	return NULL;
}