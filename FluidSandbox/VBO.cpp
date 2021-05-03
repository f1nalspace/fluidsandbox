/*
======================================================================================================================
	Fluid Sandbox - VBO.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "VBO.h"

CVBO::CVBO(void):
	iboId(0),
	vboId(0),
	reservedVertexCount(0),
	reservedVertexStride(0),
	reservedIndexCount(0) {
}

CVBO::~CVBO(void) {
	Clear();
}

void CVBO::Clear() {
	if (iboId)
		glDeleteBuffers(1, &iboId);

	iboId = 0;

	if (vboId)
		glDeleteBuffers(1, &vboId);

	vboId = 0;

	reservedVertexCount = reservedIndexCount = 0;
	reservedVertexStride = 0;
}

void CVBO::BufferVertices(const GLfloat *vertices, size_t vertexSize, GLenum usage) {
	if (vboId == 0) {
		glGenBuffers(1, &vboId);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CVBO::BufferIndices(const GLuint *indices, GLuint count, GLenum usage) {
	if (iboId == 0) {
		glGenBuffers(1, &iboId);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::ReserveIndices(const GLuint count, const GLenum usage) {
	if (iboId == 0) {
		glGenBuffers(1, &iboId);
	}
	reservedIndexCount = count;
	GLsizeiptr size = count * sizeof(GLuint);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::ReserveVertices(const GLuint vertexCount, const size_t vertexStride, const GLenum usage) {
	if (vboId == 0) {
		glGenBuffers(1, &vboId);
	}
	reservedVertexCount = vertexCount;
	reservedVertexStride = vertexStride;
	GLsizeiptr size = (GLsizeiptr)(reservedVertexCount * reservedVertexStride);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CVBO::SubbufferIndices(const GLuint *indices, const GLuint start, const GLuint count) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * start, sizeof(GL_UNSIGNED_INT) * count, indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	GLenum err = glGetError();
	assert(err == GL_NO_ERROR);
}

void CVBO::DrawElements(const GLenum mode, const GLuint count, const GLsizeiptr offset) {
	glDrawElements(mode, count, GL_UNSIGNED_INT, (void *)(offset));
}

void CVBO::Unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	GLenum err = glGetError();
	assert(err == GL_NO_ERROR);
}

VBOWritter CVBO::BeginWrite() {
	VBOWritter result = {};
	result.maxVertexCount = reservedVertexCount;
	result.maxIndexCount = reservedIndexCount;
	if (vboId) {
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		result.verts = (GLfloat *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
	if (iboId) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		result.indices = (GLuint *)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
	GLenum err = glGetError();
	assert(err == GL_NO_ERROR);
	return(result);
}

void CVBO::EndWrite(VBOWritter &writer) {
	if (writer.indices) {
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	if (writer.verts) {
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	GLenum err = glGetError();
	assert(err == GL_NO_ERROR);
}
