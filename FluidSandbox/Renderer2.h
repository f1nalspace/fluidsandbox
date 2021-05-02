#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include <final_platform_layer.h>

namespace fsr {
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
	// FrameBuffer
	//
	enum class FrameBufferAttachmentType: int {
		Color = 0,
		Depth,
		DepthStencil,
		Stencil,
	};

	struct FrameBufferAttachment {
		TextureID texture;
		FrameBufferAttachmentType type;
	};

	struct FrameBufferID {
		uint32_t id;

		bool operator<(const FrameBufferID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	class FrameBuffer {
	public:
		const FrameBufferID id;
	protected:
		std::vector<FrameBufferAttachment> attachments;
		uint32_t sampleCount;

		FrameBuffer(const FrameBufferID &id, const uint32_t sampleCount):
			id(id),
			sampleCount(sampleCount) {
		}
	public:
		virtual bool Init(const std::initializer_list<FrameBufferAttachment> &attachments) = 0;
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

	struct Viewport {
		float x;
		float y;
		float width;
		float height;
		float minDepth;
		float maxDepth;

		Viewport():
			x(0.0f),
			y(0.0f),
			width(0.0f),
			height(0.0f),
			minDepth(0.0f),
			maxDepth(1.0f) {
		}

		Viewport(const float x, const float y, const float width, const float height, const float minWidth = 0.0f, const float maxDepth = 1.0f):
			x(x),
			y(y),
			width(width),
			height(height),
			minDepth(minDepth),
			maxDepth(maxDepth) {
		}
	};

	struct ScissorRect {
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;

		ScissorRect():
			x(0),
			y(0),
			width(0),
			height(0) {
		}

		ScissorRect(const int32_t x, const int32_t y, const int32_t width, const int32_t height):
			x(x),
			y(y),
			width(width),
			height(height) {
		}
	};

	enum class ClearFlags: int32_t {
		None = 0,
		Color = 1 << 0,
		Depth = 1 << 1,
		Stencil = 1 << 2,
		ColorAndDepth = Color | Depth,
		DepthAndStencil = Depth | Stencil,
	};
	FPL_ENUM_AS_FLAGS_OPERATORS(ClearFlags);

	union ClearColorValue {
		glm::vec4 v4;
		float f32[4];
		int32_t s32[4];
		uint32_t u32[4];
	};

	struct ClearDepthStencilValue {
		float depth;
		uint32_t stencil;
	};

	struct ClearValue {
		ClearColorValue color;
		ClearDepthStencilValue depthStencil;
	};

	struct ClearSettings {
		ClearValue value;
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

	struct PipelineDescriptor {
		Viewport viewport;
		ScissorRect scissor;
		PipelineSettings settings;
		PipelineLayoutID layoutId;
		ShaderProgramID shaderProgramId;
		FrameBufferID frameBuffertId;
		PrimitiveMode primitive;
	};

	struct Pipeline {
		Viewport viewport;
		ScissorRect scissor;
		PipelineSettings settings;
		PipelineLayoutID layoutId;
		ShaderProgramID shaderProgramId;
		FrameBufferID frameBufferId;
		PrimitiveMode primitive;
		PipelineID id;

		Pipeline(const PipelineID &id):
			viewport(Viewport {}),
			scissor(ScissorRect {}),
			settings(PipelineSettings {}),
			layoutId(PipelineLayoutID {}),
			shaderProgramId(ShaderProgramID {}),
			frameBufferId(FrameBufferID {}),
			primitive(PrimitiveMode::TriangleList),
			id(id) {

		}
	};

	struct RenderPassID {
		uint32_t id;

		bool operator<(const PipelineID &o)  const {
			bool result = id < o.id;
			return(result);
		}
	};

	struct RenderArea {
		float x;
		float y;
		float width;
		float height;
	};

	struct RenderPass {
		std::vector<ClearValue> clearValues;
		RenderArea renderArea;
		FrameBufferID frameBufferId;
		RenderPassID id;

		RenderPass():
			renderArea(RenderArea {}),
			frameBufferId(FrameBufferID {}),
			id(RenderPassID {}) {

		}
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
		virtual void BindPipeline(const PipelineID &pipelineId) = 0;
		virtual void SetViewport(const float x, const float y, const float width, const float height, const float minDepth = 0.0f, const float maxDepth = 1.0f) = 0;
		virtual void SetScissor(const int x, const int y, const int width, const int height) = 0;
		virtual void BindVertexBuffers(const std::initializer_list<const BufferID> &ids) = 0;
		virtual void BindIndexBuffers(const std::initializer_list<const BufferID> &ids) = 0;
		virtual void BeginRenderPass(const RenderPassID &renderPassId, const FrameBufferID &frameBufferId, const RenderArea *renderArea, const std::initializer_list<const ClearValue> &clearValues) = 0;
		virtual void EndRenderPass() = 0;
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

		virtual FrameBufferID CreateFrameBuffer(const std::initializer_list<FrameBufferAttachment> &attachments, const uint32_t sampleCount) = 0;
		virtual void DestroyFrameBuffer(const FrameBufferID renderTargetId) = 0;

		virtual PipelineID CreatePipeline(const PipelineDescriptor &pipelineDesc) = 0;
		virtual void DestroyPipeline(const PipelineID pipelineId) = 0;

		virtual TextureID CreateTexture2D(const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data2D) = 0;
		virtual TextureID CreateTextureCube(const TextureFormat format, const uint32_t faceWidth, const uint32_t faceHeight, const uint8_t *data2Dx6) = 0;
		virtual void DestroyTexture(const TextureID textureId) = 0;

		virtual void Present() = 0;
	};

};