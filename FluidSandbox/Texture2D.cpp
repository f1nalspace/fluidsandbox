#include "Texture2D.h"


CTexture2D::CTexture2D(GLuint target, GLint internalFormat, GLenum format, GLenum type, int width, int height)
	:CTexture(target, internalFormat, format, type)
{
	this->width = width;
	this->height = height;
	this->texMagFilter = GL_LINEAR;
	this->texMinFilter = GL_LINEAR;
}

CTexture2D::CTexture2D(GLuint target, GLint internalFormat, GLenum format, GLenum type, int width, int height, GLuint* texFilters)
	:CTexture(target, internalFormat, format, type)
{
	this->width = width;
	this->height = height;
	this->texMagFilter = texFilters[0];
	this->texMinFilter = texFilters[1];
}

CTexture2D::~CTexture2D(void)
{
}

void CTexture2D::upload(char* pixels)
{
	GLuint newid = create();
	bind();
	glTexParameteri(getTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(getTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(getTarget(), GL_TEXTURE_MAG_FILTER, texMagFilter);
	glTexParameteri(getTarget(), GL_TEXTURE_MIN_FILTER, texMinFilter);
	glTexImage2D(getTarget(), 0, getInternalFormat(), width, height, 0, getFormat(), getType(), (GLvoid*)pixels);
	unbind();
}

void CTexture2D::resize(int width, int height)
{
	release();
	this->width = width;
	this->height = height;
	create();
	upload(0);
}