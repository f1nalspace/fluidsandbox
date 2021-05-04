#pragma once

#include <vector>
#include <initializer_list>
#include <cassert>

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class IndexDataType : int32_t {
	None = 0,
	U8,
	U16,
	U32,
	U64,
	Count
};

static constexpr size_t IndexDataTypeSizes[(int)IndexDataType::Count] = {
	0,  // None
	1,  // U8
	2,  // U16
	4,  // U32
	8,  // U64
};

struct IndexBufferArray {
	const size_t capacity; // Maximum number of array elements
	const size_t stride;   // The stride for each index element in bytes
	const size_t size;     // Size of the array in bytes
	size_t offset;         // Start of the array, offset in bytes (Will be computed on-the-fly)
	const IndexDataType dataType;

	IndexBufferArray(const IndexDataType dataType, const size_t capacity):
		capacity(capacity),
		stride(IndexDataTypeSizes[(int)dataType]),
		size(stride *capacity),
		offset(0),
		dataType(dataType) {
	}

	IndexBufferArray(const IndexBufferArray &other):
		capacity(other.capacity),
		stride(other.stride),
		size(other.size),
		offset(other.offset),
		dataType(other.dataType) {
	}
};

struct IndexBufferLayout {
	const std::vector<IndexBufferArray> arrays;

	IndexBufferLayout(const std::initializer_list<IndexBufferArray> &arrays):
		arrays(arrays) {
	}

	IndexBufferLayout(const IndexBufferLayout &other):
		arrays(other.arrays) {
	}
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
	std::vector<IndexBufferArray> arrays;
	GLuint iboId;
	const GLenum usage;
	size_t totalSize;

	IndexBuffer(const IndexBufferLayout &layout, const size_t indexCount, const GLenum usage, const void *data = nullptr):
		arrays(layout.arrays),
		iboId(0),
		usage(usage),
		totalSize(0) {
		totalSize = 0;
		size_t offset = 0;
		for (size_t i = 0; i < arrays.size(); ++i) {
			IndexBufferArray &arr = arrays[i];
			arr.offset = offset;
			offset += arr.size;
			totalSize += arr.size;
		}
		assert(totalSize > 0);
		glGenBuffers(1, &iboId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalSize, data, usage);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	~IndexBuffer() {
		glDeleteBuffers(1, &iboId);
	}

	void Fill(const size_t size, const void *data) {
		assert(size <= totalSize);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalSize, data, usage);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void FillPartial(const size_t arraySlot, const size_t indexOffset, const size_t indexCount, const void *data) {
		assert(arraySlot < arrays.size());
		const IndexBufferArray &arr = arrays[arraySlot];

		assert((indexOffset + indexCount) <= arr.capacity);

		size_t offset = arr.offset + indexOffset * arr.stride;
		size_t size = indexCount * arr.stride;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void Bind() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
	}

	void Unbind() {
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

