#include "GLSL.h"


CGLSL::CGLSL(void)
{
	program = glCreateProgram();
}


CGLSL::~CGLSL(void)
{
	if (program > 0)
		glDeleteProgram(program);
}

void CGLSL::enable()
{
	if (program > 0)
		glUseProgram(program);
}

void CGLSL::disable()
{
	glUseProgram(0);
}

void CGLSL::attachShader(GLuint shaderType, const char* source)
{
	GLint r = GL_FALSE;

	GLuint sid = glCreateShader(shaderType);
	glShaderSource(sid, 1, &source, NULL);
	glCompileShader(sid);
	glGetShaderiv(sid, GL_COMPILE_STATUS, &r);
	if (r == GL_FALSE)
		printShaderInfoLog(sid);

	glAttachShader(program,sid);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &r);
	if (r == GL_FALSE)
		printProgramInfoLog(program);
	
	glDeleteShader(sid);
}

void CGLSL::printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("GLSL Shader compile error: %s\n",infoLog);
		free(infoLog);
	}
}

void CGLSL::printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
	    infoLog = (char *)malloc(infologLength);
	    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("GLSL Program link error: %s\n",infoLog);
	    free(infoLog);
	}
}

GLint CGLSL::getUniformLocation(const char* name)
{
	if (program > 0)
		return glGetUniformLocation(program, name);
	else
		return 0;
}

GLint CGLSL::getAttribLocation(const char* name)
{
	if (program > 0)
		return glGetAttribLocation(program, name);
	else
		return 0;
}

void CGLSL::uniform1i(GLint location, GLint value)
{
	glUniform1i(location, value);
}

void CGLSL::uniform1f(GLint location, GLfloat value)
{
	glUniform1f(location, value);
}

void CGLSL::uniform2f(GLint location, GLfloat x, GLfloat y)
{
	glUniform2f(location, x, y);
}

void CGLSL::uniform3f(GLint location, const GLfloat* value)
{
	glUniform3fv(location, 1, value);
}

void CGLSL::uniform4f(GLint location, const GLfloat* value)
{
	glUniform4fv(location, 1, value);
}

void CGLSL::uniformMatrix4(GLint location, const GLfloat* value)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void CGLSL::enableVertexAttribArray(GLint location)
{
	glEnableVertexAttribArray(location);
}

void CGLSL::disableVertexAttribArray(GLint location)
{
	glDisableVertexAttribArray(location);
}

void CGLSL::vertexAttribPointer(GLint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid* pointer)
{
	glVertexAttribPointer(location, size, type, normalized, stride, pointer);
}
