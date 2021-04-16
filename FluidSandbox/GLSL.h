#pragma once

#include <glad/glad.h>

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
	void attachShader(const GLuint shaderType, const char* source);
	GLint getUniformLocation(const char* name);
	GLint getAttribLocation(const char* name);
	void uniform1i(const GLint location, const GLint value);
	void uniform1f(const GLint location, const GLfloat value);
	void uniform2f(const GLint location, const GLfloat x, const GLfloat y);
	void uniform3f(const GLint location, const GLfloat* value);
	void uniform4f(const GLint location, const GLfloat* value);
	void uniformMatrix4(GLint location, const GLfloat* value);
	void enableVertexAttribArray(const GLint location);
	void disableVertexAttribArray(const GLint location);
	void vertexAttribPointer(const GLint location, const GLint size, const GLenum type, const GLboolean normalized, const GLsizei stride, const GLvoid* pointer);
};

