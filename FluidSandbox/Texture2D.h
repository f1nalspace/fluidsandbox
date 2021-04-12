#pragma once
#include <GL/glew.h>
#include "Texture.h"
class CTexture2D: public CTexture
{
private:
	GLint width;
	GLint height;
	GLuint texMagFilter;
	GLuint texMinFilter;
public:
	CTexture2D(GLuint target, GLint internalFormat, GLenum format, GLenum type, int width, int height);
	CTexture2D(GLuint target, GLint internalFormat, GLenum format, GLenum type, int width, int height, GLuint* texFilters);
	~CTexture2D(void);
	virtual void upload(const char* pixels);
	void resize(int width, int height);
	GLuint getTexMagFilter() { return texMagFilter; }
	GLuint getTexMinFilter() { return texMinFilter; }
	void setTexMagFilter(GLuint texMagFilter) { this->texMagFilter = texMagFilter; }
	void setTexMinFilter(GLuint texMinFilter) { this->texMinFilter = texMinFilter; }
	GLint getWidth() { return width; }
	GLint getHeight() { return height; }
};

