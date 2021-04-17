/*
======================================================================================================================
	Fluid Sandbox - Texture.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <assert.h>

#include <glad/glad.h>

class CTexture
{
private:
	GLuint id;
	GLuint target;
	GLint internalFormat;
	GLenum format;
	GLenum type;
	GLuint userData;
	bool manualID;
protected:
	virtual GLuint create();
	void release();
public:
	CTexture(const GLuint target, const GLint internalFormat, const GLenum format, const GLenum type);
	~CTexture(void);
	void bind();
	void unbind();
	inline GLuint getTarget() { return target; };
	inline GLint getInternalFormat() { return internalFormat; };
	inline GLenum getFormat() { return format; };
	inline GLenum getType() { return type; };
	inline GLuint getUserData() { return userData; };
	inline void setUserData(GLuint value) { userData = value; };
	inline GLuint getID() { return id; };
	inline void setID(const GLuint id) { this->id = id; this->manualID = true; };
};

