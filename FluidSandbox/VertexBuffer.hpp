#pragma once

#include <vector>
#include <string>
#include <initializer_list>

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class VertexBufferDataType: int32_t {
	None = 0,
	U8,
	S16,
	U16,
	S32,
	U32,
	F32,
	V2f,
	V2i,
	V3f,
	V4f,
	Count
};

static constexpr size_t VertexBufferDataTypeSizes[(int)VertexBufferDataType::Count] = {
	0,  // None
	1,  // U8
	2,  // S16
	2,  // U16
	4,  // S32
	4,  // U32
	4,  // F32
	8,  // V2f
	8,  // V2i
	12, // V3f
	16, // V4f
};

static constexpr GLenum OpenGLVertexBufferDataTypes[(int)VertexBufferDataType::Count] = {
	0,                 // None
	GL_UNSIGNED_BYTE,  // U8
	GL_SHORT,          // S16
	GL_UNSIGNED_SHORT, // U16
	GL_INT,            // S32
	GL_UNSIGNED_INT,   // U32
	GL_FLOAT,          // F32
	GL_FLOAT,          // V2f
	GL_INT,            // V2i
	GL_FLOAT,          // V3f
	GL_FLOAT,          // V4f
};

static constexpr GLenum OpenGLVertexBufferComponentCount[(int)VertexBufferDataType::Count] = {
	0, // None
	1, // U8
	1, // S16
	1, // U16
	1, // S32
	1, // U32
	1, // F32
	2, // V2f
	2, // V2i
	3, // V3f
	4, // V4f
};

// Maps to GLSL -> layout(location = 0) out vec3 fragColor;
struct VertexBufferLayoutElement {
	const std::string name;              // Mapping name -> Not variable name!
	const size_t location;               // Location index
	const size_t size;                   // Size in bytes
	const VertexBufferDataType dataType; // The data type
	const int32_t isNormalized;          // Need the values to be normalized

	VertexBufferLayoutElement(const std::string &name, const VertexBufferDataType dataType, const size_t location, const int32_t isNormalized):
		name(name),
		location(location),
		size(VertexBufferDataTypeSizes[(int)dataType]),
		dataType(dataType),
		isNormalized(isNormalized) {
	}

	VertexBufferLayoutElement(const VertexBufferLayoutElement &other):
		VertexBufferLayoutElement(other.name, other.dataType, other.location, other.isNormalized) {
	}
};

struct VertexBufferLayout {
	const std::vector<VertexBufferLayoutElement> elements;
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

struct VertexBufferElement {
	const std::string name;       // Mapping name -> Not variable name!
	const size_t location;        // Location index
	const size_t size;            // Size in bytes
	const size_t offset;          // Offset in bytes (Will be computed on-the-fly)
	const GLenum dataType;        // OpenGL data type
	const uint32_t components;    // Number of components
	const GLboolean isNormalized; // Requires to normalize the values

	VertexBufferElement(const std::string &name, const size_t location, const size_t size, const size_t offset, const GLenum dataType, const uint32_t components, const GLboolean isNormalized):
		name(name),
		location(location),
		size(size),
		offset(offset),
		dataType(dataType),
		components(components),
		isNormalized(isNormalized) {
	}

	VertexBufferElement(const VertexBufferLayoutElement &other, const size_t offset):
		VertexBufferElement(
		other.name,
		other.location,
		other.size,
		offset,
		OpenGLVertexBufferDataTypes[(int)other.dataType],
		OpenGLVertexBufferComponentCount[(int)other.dataType],
		other.isNormalized) {
	}
};

// Deinterleaved Vertex Buffer (vec4, vec2, float, ...), (vec4, vec2, float, ...), (vec4, vec2, float, ...)
class VertexBuffer {
public:
	std::vector<VertexBufferElement> elements;
	size_t stride;
	size_t maxVertexCount;
	GLuint vboId;
	GLenum usage;
	size_t totalSize;

	VertexBuffer(const VertexBufferLayout &layout, const size_t vertexCount, const GLenum usage, const void *data = nullptr):
		stride(0),
		maxVertexCount(vertexCount),
		vboId(0),
		usage(usage),
		totalSize(0) {

		stride = 0;
		size_t offset = 0;
		for(size_t i = 0; i < layout.elements.size(); ++i) {
			const VertexBufferLayoutElement &layoutElement = layout.elements[i];
			VertexBufferElement dstElement = VertexBufferElement(layoutElement, offset);
			stride += dstElement.size;
			offset += dstElement.size;
			elements.push_back(dstElement);
		}

		glGenBuffers(1, &vboId);

		totalSize = stride * vertexCount;
		if(vertexCount > 0) {
			if(data != nullptr) {
				Fill(vertexCount, data);
			} else {
				Allocate(vertexCount);
			}
		}
	}

	~VertexBuffer() {
		glDeleteBuffers(1, &vboId);
	}

	void Bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
	}

	void Unbind() const {
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

	void Fill(const size_t count, const void *data) {
		assert(count <= maxVertexCount);
		size_t size = stride * count;
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		glBufferData(GL_ARRAY_BUFFER, size, data, usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void FillPartial(const size_t index, const size_t count, const void *data) {
		assert((index + count) <= maxVertexCount);
		size_t offset = index * stride;
		size_t size = stride * count;
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