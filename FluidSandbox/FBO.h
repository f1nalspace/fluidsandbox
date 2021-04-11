#pragma once

#include <iostream>
#include <GL/glew.h>
#include <map>
#include <assert.h>
#include <malloc.h>
#include "Texture2D.h"
using namespace std;

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) == 0;
  }
};

class CFBO
{
private:
	GLuint bufferId;
	int maxColorAttachments;
	map<const char*, CTexture2D*> textures;
	int width;
	int height;
public:
	CFBO(int width, int height);
	~CFBO(void);
	void addTextureTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, const char* name, GLuint texfilter);
	void addRenderTarget(GLint internalFormat, GLenum format, GLenum type, GLenum fbotype, const char* name, GLuint texfilter);
	CTexture2D* getTexture(const char* name);
	void enable();
	void disable();
	void resize(int width, int height);
	void update();
	void changeDepthTexture(const char* name);
	void removeDepthTexture();
	int getWidth() { return width; }
	int getHeight() { return height; }
	GLint getDrawBuffer();
	void setDrawBuffer(GLenum buffer);
	void setDrawBuffers(GLenum* buffers, int count);
	static int getMaxColorAttachments();
};

