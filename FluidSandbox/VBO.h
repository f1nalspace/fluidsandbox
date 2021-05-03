/*
======================================================================================================================
	Fluid Sandbox - VBO.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct VBOPointer {
	GLfloat *verts;
	GLuint *indices;
};

struct CVBO
{
	GLuint vboId;
	GLuint iboId;
	CVBO(void);
	~CVBO(void);
	void Clear();
	void BufferVertices(const GLfloat* vertices, const GLsizeiptr vertexSize, const GLenum usage);
	void BufferIndices(const GLuint* indices, const GLuint count, const GLenum usage);
	void ReserveIndices(const GLuint count, const GLenum usage);
	void SubbufferIndices(const GLuint* indices, const GLuint start, const GLuint count);
	void Bind();
	void Unbind();
	void DrawElements(const GLenum mode, const GLuint count, const GLsizeiptr offset);
	VBOPointer Map();
	void UnMap(VBOPointer *ptr);
};

