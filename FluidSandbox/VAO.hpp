#pragma once

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

#include <glad/glad.h>

class VAO {
private:
	GLuint vaoId;
public:
	VAO(const VertexBuffer &vertexBuffer, const IndexBuffer &indexBuffer):
		vaoId(0) {
		glGenVertexArrays(1, &vaoId);
		glBindVertexArray(vaoId);

		vertexBuffer.Bind();
		indexBuffer.Bind();

		size_t stride = vertexBuffer.stride;
		for (size_t i = 0; i < vertexBuffer.elements.size(); ++i) {
			const VertexBufferElement &element = vertexBuffer.elements[i];
			GLuint locationOrIndex = element.location;
			GLint components = element.components;
			GLenum type = element.dataType;
			GLboolean isNormalized = element.isNormalized;
			size_t offset = element.offset;
			glEnableVertexAttribArray(locationOrIndex);
			glVertexAttribPointer(locationOrIndex, components, type, isNormalized, stride, (const void *)(0 + offset));
		}

		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}

	~VAO() {
		if (vaoId > 0)
			glDeleteVertexArrays(1, &vaoId);
	}
};