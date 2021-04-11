#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

class CVBO
{
private:
	GLuint vbo;
	GLuint ibo;
	GLuint indexCount;
public:
	CVBO(void);
	~CVBO(void);
	void clear();
	void bufferVertices(GLfloat* vertices, GLuint vertexSize, GLenum usage);
	void bufferIndices(GLuint* indices, GLuint indexCount, GLenum usage);
	void bind();
	void unbind();
	void drawElements(GLenum mode);
};

