#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct CVBO
{
	GLuint vboId;
	GLuint iboId;
	CVBO(void);
	~CVBO(void);
	void clear();
	void bufferVertices(const GLfloat* vertices, const GLsizeiptr vertexSize, const GLenum usage);
	void bufferIndices(const GLuint* indices, const GLuint count, const GLenum usage);
	void reserveIndices(const GLuint count, const GLenum usage);
	void subbufferIndices(const GLuint* indices, const GLuint start, const GLuint count);
	void bind();
	void unbind();
	void drawElements(const GLenum mode, const GLuint count, const GLsizeiptr offset);
};

