#include "VBO.h"

CVBO::CVBO(void):
	iboId(0),
	vboId(0),
	indexCount(0) {
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
	indexCount = 0;
}

void CVBO::bufferVertices(const GLfloat *vertices, GLsizeiptr vertexSize, GLenum usage) {
	if(vboId == 0) {
		glGenBuffers(1, &vboId);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, vertices, usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CVBO::bufferIndices(const GLuint *indices, GLuint indexCount, GLenum usage) {
	if(iboId == 0) {
		glGenBuffers(1, &iboId);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	this->indexCount = indexCount;
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

void CVBO::drawElements(const GLenum mode, const GLuint count) {
	glDrawElements(mode, count, GL_UNSIGNED_INT, nullptr);
}

void CVBO::unbind() {
	/*
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	*/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

