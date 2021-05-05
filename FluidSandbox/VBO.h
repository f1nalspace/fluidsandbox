/*
======================================================================================================================
	Fluid Sandbox - VBO.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <glm/glm.hpp>

#include <final_dynamic_opengl.h>

struct VBOWritter {
	GLfloat *verts;
	GLuint *indices;
	GLuint vertexOffset;
	GLuint indexOffset;
	GLuint maxVertexCount;
	GLuint maxIndexCount;
};

struct CVBO
{
	size_t reservedVertexStride;
	GLuint reservedVertexCount;
	GLuint reservedIndexCount;
	GLuint vboId;
	GLuint iboId;
	CVBO(void);
	~CVBO(void);
	void Clear();
	void BufferVertices(const GLfloat* vertices, const size_t vertexSize, const GLenum usage);
	void BufferIndices(const GLuint* indices, const GLuint count, const GLenum usage);
	void ReserveIndices(const GLuint count, const GLenum usage);
	void ReserveVertices(const GLuint vertexCount, const size_t vertexStride, const GLenum usage);
	void SubbufferIndices(const GLuint* indices, const GLuint start, const GLuint count);
	void Bind();
	void Unbind();
	void DrawElements(const GLenum mode, const GLuint count, const GLsizeiptr offset);
	VBOWritter BeginWrite();
	void EndWrite(VBOWritter &writer);
};

