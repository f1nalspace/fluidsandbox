#pragma once

#include <vector>
#include <string>
#include <initializer_list>

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class VertexBufferDataType: int32_t {
	None = 0,
	S32,
	U32,
	F32,
	V2f,
	V3f,
	V4f,
	Mat2f,
	Mat3f,
	Mat4f,
	Count
};

static constexpr size_t VertexBufferDataTypeSizes[(int)VertexBufferDataType::Count] = {
	0,  // None
	4,  // S32
	4,  // U32
	4,  // F32
	8,  // V2f
	12, // V3f
	16, // V4f
	16, // Mat2f
	36, // Mat3f
	64, // Mat4f
};

// Maps to GLSL -> layout(location = 0) out vec3 fragColor;
struct VertexBufferLayoutElement {
	const std::string name; // Mapping name -> Not variable name!
	const size_t location;  // Location index
	const size_t size;      // Size in bytes
	size_t offset;          // Offset in bytes (Will be computed on-the-fly)
	VertexBufferDataType dataType;

	VertexBufferLayoutElement():
		name(""),
		location(0),
		size(0),
		dataType(VertexBufferDataType::None),
		offset(0) {
	}

	VertexBufferLayoutElement(const std::string &name, const VertexBufferDataType dataType, const size_t location):
		name(name),
		location(location),
		size(VertexBufferDataTypeSizes[(int)dataType]),
		dataType(dataType),
		offset(0) {
	}

	VertexBufferLayoutElement(const VertexBufferLayoutElement &other):
		name(other.name),
		location(other.location),
		size(other.size),
		dataType(other.dataType),
		offset(other.offset) {
	}
};

struct VertexBufferLayout {
	std::vector<VertexBufferLayoutElement> elements;
	VertexBufferLayout(const std::initializer_list<VertexBufferLayoutElement> &elements):
		elements(elements) {
	}
	VertexBufferLayout(const VertexBufferLayout &layout):
		elements(layout.elements) {
	}
};

struct VertexBufferWriter {
	void *base;
	size_t writtenVertexCount;
	const size_t vertexStride;
	const size_t maxVertexCount;

	VertexBufferWriter(void *base, const size_t vertexStride, const size_t maxVertexCount):
		base(base),
		writtenVertexCount(0),
		vertexStride(vertexStride),
		maxVertexCount(maxVertexCount) {
	}
};

// Deinterleaved Vertex Buffer (vec4, vec2, float, ...), (vec4, vec2, float, ...), (vec4, vec2, float, ...)
class VertexBuffer {
public:
	std::vector<VertexBufferLayoutElement> elements;
	size_t stride;
	size_t maxVertexCount;
	GLuint vboId;
	GLenum usage;
	size_t totalSize;

	VertexBuffer(const VertexBufferLayout &layout, const size_t vertexCount, const GLenum usage, const void *data = nullptr):
		elements(layout.elements),
		stride(0),
		maxVertexCount(vertexCount),
		vboId(0),
		usage(usage),
		totalSize(0) {
		stride = 0;
		size_t offset = 0;
		for (size_t i = 0; i < elements.size(); ++i) {
			VertexBufferLayoutElement &element = elements[i];
			VertexBufferDataType dataType = element.dataType;
			size_t size = VertexBufferDataTypeSizes[(int)dataType];
			element.offset = offset;
			stride += size;
			offset += size;
		}
		totalSize = stride * vertexCount;
		glGenBuffers(1, &vboId);

		if (vertexCount > 0) {
			if (data != nullptr) {
				Fill(totalSize, data);
			} else {
				Allocate(vertexCount);
			}
		}
	}

	~VertexBuffer() {
		glDeleteBuffers(1, &vboId);
	}

	void Bind() {
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
	}

	void Unbind() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Allocate(const size_t vertexCount) {
		maxVertexCount = vertexCount;
		totalSize = stride * maxVertexCount;
		size_t bufferSize = stride * vertexCount;
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Fill(const size_t size, const void *data) {
		assert(maxVertexCount > 0);
		size_t bufferSize = stride * maxVertexCount;
		assert(size <= bufferSize);
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferData(GL_ARRAY_BUFFER, bufferSize, data, usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void FillPartial(const size_t offset, const size_t size, const void *data) {
		assert(maxVertexCount > 0);
		size_t bufferSize = stride * maxVertexCount;
		assert((offset + size) <= bufferSize);
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	VertexBufferWriter BeginWrite() {
		Bind();
		void *data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		VertexBufferWriter result = VertexBufferWriter(data, stride, maxVertexCount);
		return(result);
	}

	void EndWrite(VertexBufferWriter &writer) {
		assert(writer.writtenVertexCount <= writer.maxVertexCount);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		Unbind();
	}
};