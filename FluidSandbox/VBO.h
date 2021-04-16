#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class CVBO
{
private:
	GLuint vboId;
	GLuint iboId;
	GLuint indexCount;
public:
	CVBO(void);
	~CVBO(void);
	void clear();
	void bufferVertices(const GLfloat* vertices, GLsizeiptr vertexSize, GLenum usage);
	void bufferIndices(const GLuint* indices, GLuint indexCount, GLenum usage);
	void bind();
	void unbind();
	void drawElements(const GLenum mode);
};

