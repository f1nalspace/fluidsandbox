#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include <final_platform_layer.h>

namespace renderer {
	typedef int32_t b32;

	//
	// Buffer
	//
	enum class BufferType: int {
		None = 0,
		Vertex,
		Index,
		Uniform,
	};

	enum class BufferUsage {
		None = 0,
		Static,
		Dynamic,
	};

	enum class BufferAccess {
		None = 0,
		ReadOnly,
		WriteOnly,
		ReadWrite,
	};

	struct BufferID {
		uint32_t id;

		bool operator<(const BufferID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	class Buffer {
	public:
		BufferID id;
		size_t size;
		BufferType type;
		BufferAccess access;
		BufferUsage usage;
	protected:
		Buffer(const BufferID id, const BufferType type, const BufferAccess access, const BufferUsage usage, const size_t size):
			id(id),
			type(type),
			access(access),
			usage(usage),
			size(size) {
		}
	public:
		virtual bool Init(const uint8_t *data) = 0;
		virtual void Release() = 0;

		virtual void Write(const size_t offset, const size_t size, const uint8_t *data) = 0;

		virtual void *Map() = 0;
		virtual void Unmap() = 0;
	};

	//
	// Texture
	//
	enum class TextureType: int {
		None = 0,
		T1D,
		T2D,
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

	class Texture {
	public:
		uint32_t width;
		uint32_t height;
		TextureID id;
		TextureFormat format;
		TextureType type;
	protected:
		Texture(const TextureID id, const TextureType type, const TextureFormat format, const uint32_t width, const uint32_t height):
			width(0),
			height(0),
			id(id),
			format(format),
			type(type) {
		}
	public:
		virtual bool Write(const size_t size, const uint8_t *data) = 0;
	};

	//
	// RenderTarget
	//
	enum class RenderTargetAttachmentType: int {
		Color = 0,
		Depth,
		DepthStencil,
		Stencil,
	};

	struct RenderTargetAttachment {
		TextureID texture;
		RenderTargetAttachmentType type;
	};

	struct RenderTargetID {
		uint32_t id;

		bool operator<(const RenderTargetID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	struct RenderTargetParams {
		constexpr static uint32_t MaxAttachmentCount = 16;
		RenderTargetAttachment attachments[MaxAttachmentCount];
		uint32_t attachmentCount;
		uint32_t sampleCount;
	};

	class RenderTarget {
	public:
		const RenderTargetID id;
	protected:
		RenderTarget(const RenderTargetID &id):
			id(id) {
		}
	public:
		virtual bool Init(const RenderTargetParams &params) = 0;
		virtual void Release() = 0;
	};

	//
	// Shader
	//
	// TODO(final): Add other shader types (Tessellation, Geometry, Compute)
	enum class ShaderType: int {
		None = 0,
		Vertex,
		Fragment,
		First = Vertex,
		Last = Fragment,
	};
	static constexpr uint32_t MaxShaderTypeCount = ((int)ShaderType::Last - (int)ShaderType::First) + 1;

	struct ShaderID {
		uint32_t id;

		bool operator<(const ShaderID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	struct Shader {
		const char *source;
		ShaderID id;
		ShaderType type;
	};

	struct ShaderProgramID {
		uint32_t id;

		bool operator<(const ShaderProgramID &o)  const {
			bool result = id < o.id;
			return(result);
		}
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
		Vec2,
		Vec3,
		Vec4,
		Mat2,
		Mat3,
		Mat4,
		Int,
		Vec2i,
		Vec3i,
		Vec4i,
		Sampler2D,
		SamplerCube,
		First = Float,
		Last = SamplerCube,
	};

	static constexpr uint32_t MaxUniformTypeCount = ((int)UniformType::Last - (int)UniformType::First) + 1;

	struct UniformID {
		uint32_t id;

		bool operator<(const UniformID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	struct Uniform {
		const char *name;
		uint32_t offset;
		uint32_t size;
		UniformID id;
		UniformType type;
	};

	//
	// LayoutLocationElement
	//
	enum class LayoutLocationElementType: int {
		None = 0,
		Float,
		Vec2f,
		Vec3f,
		Vec4f,
	};

	struct LayoutLocationElementID {
		uint32_t id;

		bool operator<(const LayoutLocationElementID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

		// Mirrors the layout(location = offset) in/out type name
	struct LayoutLocationElement {
		const char *name;
		uint32_t offset;
		uint32_t count;
		uint32_t size;
		LayoutLocationElementID id;
		LayoutLocationElementType type;
		b32 isNormalized;
	};

	//
	// Pipeline
	//
	struct PipelineLayoutID {
		uint32_t id;

		bool operator<(const PipelineLayoutID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	struct PipelineLayout {
		std::vector<Uniform> uniforms;
		std::vector<LayoutLocationElement> locationElements;
		PipelineLayoutID id;
	};

	enum class PrimitiveMode: int {
		PointList = 0,
		LineList,
		LineStrip,
		LineLoop,
		TriangleList,
		TriangleStrip,
		TriangleFan,
	};

	struct ClipRect {
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;

		ClipRect():
			x(0),
			y(0),
			width(0),
			height(0) {
		}

		ClipRect(const int x, const int y, const int width, const int height):
			x(x),
			y(y),
			width(width),
			height(height) {

		}
	};

	enum class ClearFlags: uint32_t {
		None = 0,
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2,
	};
	FPL_ENUM_AS_FLAGS_OPERATORS(ClearFlags);

	struct ClearSettings {
		float color[4];
		ClearFlags flags;
	};

	struct ColorSettings {
		b32 writeEnabled[4];
	};

	enum class DepthTest: int {
		Off = 0,
		On = 1
	};

	enum class DepthFunc: int {
		Never = 0,
		Equal,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual,
		Always,
	};

	struct DepthSettings {
		float clearDepth;
		b32 writeEnabled;
		DepthTest test;
		DepthFunc func;
	};

	enum class BlendOp: int {
		Zero = 0,
		One,
		SrcColor,
		InvSrcColor,
		SrcAlpha,
		InvSrcAlpha,
		DstColor,
		InvDstColor,
		DstAlpha,
		InvDstAlpha,
	};

	struct BlendSettings {
		BlendOp sourceColor;
		BlendOp destColor;
		b32 isEnabled;
	};

	enum class CullMode: int {
		None = 0,
		ClockWise,
		CounterClockWise,
	};

	enum class PolygonMode: int {
		Fill = 0,
		Line,
		Point
	};

	struct PipelineSettings {
		ClearSettings clear;
		ColorSettings color;
		DepthSettings depth;
		BlendSettings blend;
		CullMode cullMode;
		PolygonMode polygonMode;
	};

	struct PipelineID {
		uint32_t id;

		bool operator<(const PipelineID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	struct Pipeline {
		ClipRect viewport;
		ClipRect scissor;
		PipelineSettings pipelineSettings;
		PipelineLayoutID pipelineLayout;
		ShaderProgramID shaderProgram;
		RenderTargetID renderTarget;
		PrimitiveMode primitive;
		PipelineID id;
	};

	enum class RendererType: int {
		None = 0,
		Null,
		OpenGL
	};

	class CommandBuffer {
	public:
		virtual bool Begin() = 0;
		virtual void End() = 0;
		virtual void SetViewport(const int x, const int y, const int width, const int height) = 0;
		virtual void SetScissor(const int x, const int y, const int width, const int height) = 0;
		virtual void SetPipeline(const PipelineID &pipelineId) = 0;
		virtual void SetBuffer(const BufferID &bufferId) = 0;
		virtual void SetTexture(const TextureID &textureId, const uint32_t index) = 0;
		virtual void SetUniform(const UniformID &uniformId, const size_t size, const uint8_t *data) = 0;
		virtual void Draw(const size_t vertexCount, const size_t firstVertex = 0, const size_t instanceCount = 1, const size_t firstInstance = 0) = 0;
	};

	class CommandQueue {
	public:
		virtual bool Submit(CommandBuffer &commandBuffer) = 0;
	};

	class Renderer {
	protected:
		virtual bool Init() = 0;
		virtual void Release() = 0;
	public:
		static Renderer *Create(const RendererType type);

		virtual CommandQueue *GetCommandQueue() = 0;

		virtual CommandBuffer *CreateCommandBuffer() = 0;
		virtual void DestroyCommandBuffer(CommandBuffer *commandBuffer) = 0;

		virtual BufferID CreateBuffer(const BufferType type, const BufferAccess access, const BufferUsage usage, const size_t size, const uint8_t *data) = 0;
		virtual void DestroyBuffer(const BufferID bufferId) = 0;

		virtual RenderTargetID CreateRenderTarget(const RenderTargetParams &params) = 0;
		virtual void DestroyRenderTarget(const RenderTargetID renderTargetId) = 0;

		virtual PipelineID CreatePipeline() = 0;
		virtual void DestroyPipeline(const PipelineID pipelineId) = 0;

		virtual TextureID CreateTexture2D(const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data2D) = 0;
		virtual TextureID CreateTextureCube(const TextureFormat format, const uint32_t faceWidth, const uint32_t faceHeight, const uint8_t *data2Dx6) = 0;
		virtual void DestroyTexture(const TextureID textureId) = 0;

		virtual void Present() = 0;
	};

};