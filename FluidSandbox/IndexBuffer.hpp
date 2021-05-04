/*
======================================================================================================================
	Fluid Sandbox - IndexBuffer.hpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <vector>
#include <initializer_list>
#include <cassert>

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class IndexDataType: int32_t {
	None = 0,
	U8,
	U16,
	U32,
	Count
};

static constexpr GLuint IndexDataTypeSizes[(int)IndexDataType::Count] = {
	0,  // None
	1,  // U8
	2,  // U16
	4,  // U32
};

static constexpr GLenum GLIndexDataTypeFormats[(int)IndexDataType::Count] = {
	0,                 // None
	GL_UNSIGNED_BYTE,  // U8
	GL_UNSIGNED_SHORT, // U16
	GL_UNSIGNED_INT,   // U32
};

struct IndexBufferWriter {
	void *base;
	size_t writtenSize;
	const size_t maxSize;

	IndexBufferWriter(void *base, const size_t maxSize):
		base(base),
		writtenSize(0),
		maxSize(maxSize) {
	}
};

// Interleaved Index Buffer (A, A, A, A), (B, B, B, B, B, B)
class IndexBuffer {
public:
	const size_t stride;
	size_t totalSize;
	size_t maxIndexCount;
	const GLenum type;
	const GLenum usage;
	GLuint iboId;

	IndexBuffer(const IndexDataType dataType, const size_t indexCount, const GLenum usage, const void *data = nullptr):
		stride(IndexDataTypeSizes[(int)dataType]),
		totalSize(0),
		maxIndexCount(indexCount),
		type(GLIndexDataTypeFormats[(int)dataType]),
		usage(usage),
		iboId(0) {
		glGenBuffers(1, &iboId);

		totalSize = stride * indexCount;
		if (indexCount > 0) {
			if (data != nullptr) {
				Fill(indexCount, data);
			} else {
				Allocate(indexCount);
			}
		}
	}

	~IndexBuffer() {
		glDeleteBuffers(1, &iboId);
	}

	void Allocate(const size_t indexCount) {
		maxIndexCount = indexCount;
		totalSize = stride * maxIndexCount;
		glBindBuffer(GL_ARRAY_BUFFER, iboId);
		glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Fill(const size_t count, const void *data) {
		assert(count <= maxIndexCount);
		size_t size = count * stride;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void FillPartial(const size_t index, const size_t count, const void *data) {
		assert((index + count) <= maxIndexCount);
		size_t offset = index * stride;
		size_t size = count * stride;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void Bind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	}

	void Unbind() const {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	IndexBufferWriter BeginWrite() {
		Bind();
		void *data = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		IndexBufferWriter result = IndexBufferWriter(data, totalSize);
		return(result);
	}

	void EndWrite(IndexBufferWriter &writer) {
		assert(writer.writtenSize <= writer.maxSize);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		Unbind();
	}
};

