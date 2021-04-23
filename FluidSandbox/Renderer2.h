#pragma once

#include <cstdint>
#include <vector>
#include <memory>

//
// Buffer
//
enum class BufferType: int {
	None = 0,
	Vertex,
	Index,
	Uniform,
	Storage,
};

enum class BufferAccess: int {
	ReadWrite = 0,
	WriteOnly,
	ReadOnly,
};

enum class BufferUsage {
	Static = 0,
	Dynamic,
};

struct BufferID {
	uint32_t id;

	bool operator<(const BufferID &o)  const {
		bool result = id < o.id;
		return(result);
	}
};

struct Buffer {
	size_t size;
	BufferID id;
	BufferType type;
	BufferAccess access;
	BufferUsage usage;
protected:
	inline Buffer(const BufferID id, const BufferType type, const size_t size, const BufferAccess access, const BufferUsage usage):
		size(size),
		id(id),
		type(type),
		access(access),
		usage() {
	}
};

//
// Texture
//
enum class TextureType: int {
	None = 0,
	T1D,
	T2D,
	T3D,
	Cube,
};

enum class TextureFormat: int {
	//! No texture format
	None = 0,

	//! Unsigned byte, 1 byte per channel, Alpha channels only
	AlphaU8,

	//! Unsigned byte, 1 byte per channel, RGBA channels
	RGBAU8,

	//! Floating point, 4-bytes per channel, RGBA channels
	RGBAF32,
};

struct TextureID {
	uint32_t id;

	bool operator<(const TextureID &o)  const {
		bool result = id < o.id;
		return(result);
	}
};

struct Texture {
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	TextureID id;
	TextureFormat format;
	TextureType type;
protected:
	inline Texture(const TextureID id, const TextureType type, const TextureFormat format, const uint32_t width, const uint32_t height, const uint32_t depth):
		width(0),
		height(0),
		depth(0),
		id(id),
		format(format),
		type(type) {
	}
};

//
// RenderTarget
//
struct RenderTargetID {
	uint32_t id;
};

struct RenderTarget {
	TextureID texture;
	RenderTargetID id;
};

//
// Shader
//
enum class ShaderType: int {
	None = 0,
	Vertex,
	Geometry,
	Fragment,
	Compute,
	First = Vertex,
	Last = Compute,
};

struct ShaderID {
	uint32_t id;
};

struct Shader {
	const char *source;
	ShaderID id;
	ShaderType type;
};

static constexpr uint32_t MaxShaderTypeCount = ((int)ShaderType::Last - (int)ShaderType::First) + 1;

struct ShaderProgramID {
	uint32_t id;
};

struct ShaderProgram {
	ShaderID shaders[MaxShaderTypeCount];
	ShaderProgramID id;
	uint32_t shaderCount;
};

//
// Uniform
//
enum class UniformType: int {
	None = 0,
	Float,
	Vec2f,
	Vec3f,
	Vec4f,
	Mat3f,
	Mat4f,
	Int,
	Vec2i,
	Vec3i,
	Vec4i,
	First = Float,
	Last = Vec4i,
};

static constexpr uint32_t MaxUniformTypeCount = ((int)UniformType::Last - (int)UniformType::First) + 1;

struct UniformID {
	uint32_t id;
};

struct Uniform {
	const char *name;
	size_t offset;
	size_t size;
	UniformID id;
	UniformType type;
};

//
// BufferLayoutElement
//
enum class BufferLayoutElementType: int {
	None = 0,
	Float1,
	Float2,
	Float3,
	Float4,
	Int32,
	Byte1,
	Byte2,
	Byte3,
	Byte4,
};

struct BufferLayoutElementID {
	uint32_t id;
};

struct BufferLayoutElement {
	const char *name;
	uint32_t offset;
	uint32_t count;
	uint32_t size;
	BufferLayoutElementID id;
	BufferLayoutElementType type;
	bool isNormalized;
};

//
// Pipeline
//
enum class PipelineLayoutBindingType: int {
	None = 0,
	UniformBuffer,
	StorageBuffer,
	Sampler
};

struct PipelineLayoutBindingID {
	uint32_t id;
};

struct PipelineLayoutBinding {
	size_t offset;
	size_t size;
	PipelineLayoutBindingID id;
	PipelineLayoutBindingType type;
};

struct PipelineLayoutID {
	uint32_t id;
};

struct PipelineLayout {
	std::vector<UniformID> uniforms;
	std::vector<PipelineLayoutBindingID> layoutBindings;
	std::vector<BufferLayoutElementID> vertexLayouts;
	PipelineLayoutID id;
};

enum class DepthTest: int {
	Off = 0,
	On = 1
};

enum class CullMode: int {
	None = 0,
	ClockWise,
	CounterClockWise,
};

enum class PrimitiveMode: int {
	Triangle = 0,
	TriangleLine = 0,
	TriangleStrip = 0,
	Point = 0,
	Line = 0,
	LineStrip = 0,
};

struct RendererRect {
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

struct PipelineSettings {
	DepthTest depthTest;
	CullMode cullMode;
	PrimitiveMode drawMode;
};

struct PipelineID {
	uint32_t id;
};

struct Pipeline {
	RendererRect viewport;
	RendererRect scissor;
	PipelineSettings pipelineSettings;
	PipelineLayoutID pipelineLayout;
	ShaderProgramID shaderProgram;
	RenderTargetID renderTarget;
	PipelineID id;
};

struct SamplerID {
	uint32_t id;
};

struct Sampler {
	TextureID texture;
	SamplerID id;
};

struct BindingID {
	uint32_t id;
};

struct Binding {
	std::vector<BufferID> buffers;
	std::vector<SamplerID> samplers;
	BindingID id;
};

enum class RendererType: int {
	None = 0,
	Null,
	OpenGL
};

class Renderer {
public:
	static Renderer *Create(const RendererType type);

	virtual Buffer *CreateBuffer(const BufferType type, const size_t size, const BufferAccess access, const BufferUsage usage) = 0;
	virtual void DestroyBuffer(Buffer &buffer) = 0;

	virtual Texture *CreateTexture2D(const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data2D) = 0;
	virtual Texture *CreateTextureCube(const TextureFormat format, const uint32_t faceWidth, const uint32_t faceHeight, const uint8_t *data2Dx6) = 0;
	virtual void DestroyTexture(Texture &texture) = 0;
};