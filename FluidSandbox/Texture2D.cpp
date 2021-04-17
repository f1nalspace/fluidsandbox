#include "Texture2D.h"


CTexture2D::CTexture2D(const GLuint target, const GLint internalFormat, const GLenum format, const GLenum type, const int width, const int height, const GLuint magFilter, const GLuint minFilter)
	:CTexture(target, internalFormat, format, type) {
	this->width = width;
	this->height = height;
	this->texMagFilter = magFilter;
	this->texMinFilter = minFilter;
}

CTexture2D::~CTexture2D(void)
{
}

void CTexture2D::upload(const uint8_t* pixels)
{
	GLuint newid = create();
	bind();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(getTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(getTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(getTarget(), GL_TEXTURE_MAG_FILTER, texMagFilter);
	glTexParameteri(getTarget(), GL_TEXTURE_MIN_FILTER, texMinFilter);
	glTexImage2D(getTarget(), 0, getInternalFormat(), width, height, 0, getFormat(), getType(), (const GLvoid*)pixels);
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