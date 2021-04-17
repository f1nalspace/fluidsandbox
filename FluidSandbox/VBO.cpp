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
	clear();
}

void CVBO::clear() {
	if(iboId)
		glDeleteBuffers(1, &iboId);

	iboId = 0;

	if(vboId)
		glDeleteBuffers(1, &vboId);

	vboId = 0;
}

void CVBO::bufferVertices(const GLfloat *vertices, GLsizeiptr vertexSize, GLenum usage) {
	if(vboId == 0) {
		glGenBuffers(1, &vboId);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CVBO::bufferIndices(const GLuint *indices, GLuint count, GLenum usage) {
	if(iboId == 0) {
		glGenBuffers(1, &iboId);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), indices, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::reserveIndices(const GLuint count, const GLenum usage) {
	if(iboId == 0) {
		glGenBuffers(1, &iboId);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), nullptr, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::subbufferIndices(const GLuint *indices, const GLuint start, const GLuint count) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * start, sizeof(GL_UNSIGNED_INT) * count, indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CVBO::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	/*
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glNormalPointer(GL_FLOAT, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 6, (void*)(0));
	*/
}

void CVBO::drawElements(const GLenum mode, const GLuint count, const GLsizeiptr offset) {
	glDrawElements(mode, count, GL_UNSIGNED_INT, (void *)(offset));
}

void CVBO::unbind() {
	/*
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	*/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

