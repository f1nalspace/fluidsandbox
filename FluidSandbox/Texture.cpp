#include "Texture.h"

CTexture::CTexture(const GLuint target, const GLint internalFormat, const GLenum format, const GLenum type)
{
	this->id = 0;
	this->target = target;
	this->internalFormat = internalFormat;
	this->format = format;
	this->type = type;
	this->userData = 0;
	this->manualID = false;
}

CTexture::~CTexture(void)
{
	release();
}

void CTexture::bind() {
	glBindTexture(this->target, this->id);
}

void CTexture::unbind() {
	glBindTexture(this->target, 0);
}

GLuint CTexture::create() { 
	if (id == 0)
		glGenTextures(1, &id); 
	return id; 
}

void CTexture::release()
{
	if (id) {
		if (!manualID) {
			glDeleteTextures(1, &id);
		}
		id = 0;
	}
}
