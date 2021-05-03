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
	vboId(0) {
}

CVBO::~CVBO(void) {
	Clear();
}

void CVBO::Clear() {
	if(iboId)
		glDeleteBuffers(1, &iboId);

	iboId = 0;

	if(vboId)
		glDeleteBuffers(1, &vboId);

	vboId = 0;
}

void CVBO::BufferVertices(const GLfloat *vertices, GLsizeiptr vertexSize, GLenum usage) {
	if(vboId == 0) {
		glGenBuffers(1, &vboId);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CVBO::BufferIndices(const GLuint *indices, GLuint count, GLenum usage) {
	if(iboId == 0) {
		glGenBuffers(1, &iboId);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::ReserveIndices(const GLuint count, const GLenum usage) {
	if(iboId == 0) {
		glGenBuffers(1, &iboId);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), nullptr, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::SubbufferIndices(const GLuint *indices, const GLuint start, const GLuint count) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * start, sizeof(GL_UNSIGNED_INT) * count, indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
}

void CVBO::DrawElements(const GLenum mode, const GLuint count, const GLsizeiptr offset) {
	glDrawElements(mode, count, GL_UNSIGNED_INT, (void *)(offset));
}

void CVBO::Unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VBOPointer CVBO::Map() {
	VBOPointer result = {};
	if(vboId) {
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		result.verts = (GLfloat *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	}
	if(iboId)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		result.indices = (GLuint *)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	return(result);
}

void CVBO::UnMap(VBOPointer *ptr) {
	if(ptr->indices) {
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	if(ptr->verts) {
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}
