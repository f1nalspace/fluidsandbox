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
	enum class RenderTargetAttachmentType : int {
		None = 0,
		Depth,
		Color,
		Stencil
	};

	struct RenderTargetAttachment {
		TextureID texture;
		RenderTargetAttachmentType type;
	};

	struct RenderTargetID {
		uint32_t id;
	};

	struct RenderTarget {
		constexpr static uint32_t MaxAttachmentCount = 16; // One depth, one stencil and 14 color attachments
		union {
			struct {
				RenderTargetAttachment depth;
				RenderTargetAttachment stencil;
				RenderTargetAttachment color[14];
			};
			RenderTargetAttachment all[MaxAttachmentCount];
		} attachments;
		RenderTargetID id;
		uint32_t colorAttachmentCount;
		b32 hasDepthAttachment;
		b32 hasStencilAttachment;
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
	};

	struct PipelineLayout {
		std::vector<Uniform> uniforms;
		std::vector<LayoutLocationElement> locationElements;
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

	enum class ClearFlags: uint32_t {
		None = 0,
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2,
	};
	FPL_ENUM_AS_FLAGS_OPERATORS(ClearFlags);

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
	};

	struct Color4f {
		float r, g, b, a;
	};

	struct DepthSettings {
		b32 writeEnabled;
		DepthTest testing;
	};

	struct PipelineSettings {
		Color4f clearColor;
		DepthSettings depth;
		CullMode cullMode;
		PrimitiveMode drawMode;
		ClearFlags clearFlags;
	};

	struct PipelineID {
		uint32_t id;
	};

	struct Pipeline {
		ClipRect viewport;
		ClipRect scissor;
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
		virtual void SetPipeline(const Pipeline &pipeline) = 0;
		virtual void SetBuffer(const Buffer &buffer) = 0;
		virtual void Draw(const size_t vertexCount, const size_t firstVertex = 0, const size_t instanceCount = 1, const size_t firstInstance = 0) = 0;
	};

	class CommandQueue {
	public:
		virtual bool Submit(CommandBuffer &commandBuffer) = 0;
	};

	class Renderer {
	public:
		static Renderer *Create(const RendererType type);

		virtual CommandQueue *GetCommandQueue() = 0;

		virtual CommandBuffer *CreateCommandBuffer() = 0;
		virtual void DestroyCommandBuffer(CommandBuffer &commandBuffer) = 0;

		virtual Buffer *CreateBuffer(const BufferType type, const size_t size, const BufferAccess access, const BufferUsage usage) = 0;
		virtual void DestroyBuffer(Buffer &buffer) = 0;

		virtual Texture *CreateTexture2D(const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data2D) = 0;
		virtual Texture *CreateTextureCube(const TextureFormat format, const uint32_t faceWidth, const uint32_t faceHeight, const uint8_t *data2Dx6) = 0;
		virtual void DestroyTexture(Texture &texture) = 0;

		virtual void Present() = 0;
	};

};