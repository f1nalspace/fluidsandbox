/*
======================================================================================================================
	Fluid Sandbox - Texture2D.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <cstdint>

#include <final_dynamic_opengl.h>

#include "Texture.h"

class CTexture2D: public CTexture
{
private:
	GLint width;
	GLint height;
	GLuint texMagFilter;
	GLuint texMinFilter;
public:
	CTexture2D(const GLuint target, const GLint internalFormat, const GLenum format, const GLenum type, const int width, const int height, const GLuint magFilter = GL_LINEAR, const GLuint minFilter = GL_LINEAR);
	~CTexture2D(void);
	virtual void upload(const uint8_t *pixels);
	void resize(int width, int height);
	GLuint getTexMagFilter() { return texMagFilter; }
	GLuint getTexMinFilter() { return texMinFilter; }
	void setTexMagFilter(GLuint texMagFilter) { this->texMagFilter = texMagFilter; }
	void setTexMinFilter(GLuint texMinFilter) { this->texMinFilter = texMinFilter; }
	GLint getWidth() { return width; }
	GLint getHeight() { return height; }
};

