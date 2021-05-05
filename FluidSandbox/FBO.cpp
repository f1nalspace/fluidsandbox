/*
======================================================================================================================
	Fluid Sandbox - FBO.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "FBO.h"

#include <iostream>

CFBO::CFBO(const int width, const int height):
	width(width),
	height(height),
	bufferId(0),
	textureCount(0),
	maxColorAttachments(0) {
	for(uint32_t i = 0; i < MaxTextureCount; ++i)
		textures[i] = nullptr;

	GLint temp;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &temp);
	maxColorAttachments = temp;
}

CFBO::~CFBO(void) {
	// Remove textures
	for(uint32_t i = 0; i < textureCount; ++i)
		delete textures[i];
	textureCount = 0;

	if(bufferId)
		glDeleteFramebuffers(1, &bufferId);
}

uint32_t CFBO::getMaxColorAttachments() {
	int temp = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &temp);
	uint32_t result = temp;
	return temp;
}

bool StatusFBO() {
	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(GL_FRAMEBUFFER_COMPLETE)
		return true;
	else {
		switch(error) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				printf("    Incomplete attachment!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				printf("    Missing attachment!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				printf("    Incomplete draw buffer!\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				printf("    Incomplete read buffer!\n");
				break;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				printf("    Framebufferobjects unsupported!\n");
				break;
		}

		return false;
	}
}

CTexture2D *CFBO::addTextureTarget(const GLint internalFormat, const GLenum format, const GLenum type, const GLenum fbotype, const GLuint texfilter) {
	assert(textureCount < MaxTextureCount);

	// Create new texture
	CTexture2D *newtex = new CTexture2D(GL_TEXTURE_2D, internalFormat, format, type, width, height, texfilter, texfilter);
	newtex->setUserData(fbotype);
	newtex->upload(nullptr);

	// Add texture to array and map
	textures[textureCount++] = newtex;
	return(newtex);
}

CTexture2D *CFBO::addRenderTarget(const GLint internalFormat, const GLenum format, const GLenum type, const GLenum fbotype, const GLuint texfilter) {
	assert(textureCount < MaxTextureCount);

	// Create new texture
	CTexture2D *newtex = new CTexture2D(GL_TEXTURE_2D, internalFormat, format, type, width, height, texfilter, texfilter);
	newtex->setUserData(fbotype);
	newtex->upload(nullptr);

	// Add texture to array and map
	textures[textureCount++] = newtex;
	return(newtex);
}

void CFBO::enable() {
	if(bufferId)
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bufferId);
}

void CFBO::disable() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void CFBO::resize(const int width, const int height) {
	this->width = width;
	this->height = height;

	// Recreate textures
	for(uint32_t i = 0; i < textureCount; ++i)
		textures[i]->resize(width, height);

	// Update FBO
	update();
}

void CFBO::update() {
	// Create framebuffer if not present
	if(!bufferId)
		glGenFramebuffers(1, &bufferId);

	// Bind framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bufferId);

	// Update/Add textures to framebuffer
	for(uint32_t i = 0; i < textureCount; ++i) {
		CTexture2D *tex = textures[i];
		GLuint userdata = tex->getUserData();

		if(userdata > 0)  // We want only color buffer
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, userdata, GL_TEXTURE_2D, tex->getID(), 0);
	}

	// Check FBO Status
	StatusFBO();

	// Unbind framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

GLint CFBO::getDrawBuffer() {
	GLint savedDrawBuffer;
	glGetIntegerv(GL_DRAW_BUFFER, &savedDrawBuffer);
	return savedDrawBuffer;
}

void CFBO::setDrawBuffer(const GLenum buffer) {
	glDrawBuffer(buffer);
}

void CFBO::setDrawBuffers(const GLenum *buffers, const int count) {
	glDrawBuffers(count, buffers);
}


