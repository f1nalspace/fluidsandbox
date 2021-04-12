#pragma once

#include <GL/glew.h>

class CGLSL
{
private:
	GLuint program;
	void printShaderInfoLog(GLuint obj);
	void printProgramInfoLog(GLuint obj);
protected:
	virtual void updateUniformLocations(){}
public:
	CGLSL(void);
	~CGLSL(void);
	void enable();
	void disable();
	void attachShader(GLuint shaderType, const char* source);
	GLint getUniformLocation(const char* name);
	GLint getAttribLocation(const char* name);
	void uniform1i(GLint location, GLint value);
	void uniform1f(GLint location, GLfloat value);
	void uniform2f(GLint location, GLfloat x, GLfloat y);
	void uniform3f(GLint location, const GLfloat* value);
	void uniform4f(GLint location, const GLfloat* value);
	void uniformMatrix4(GLint location, const GLfloat* value);
	void enableVertexAttribArray(GLint location);
	void disableVertexAttribArray(GLint location);
	void vertexAttribPointer(GLint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid* pointer);
};

