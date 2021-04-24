#include "Renderer2.h"

#include <map>
#include <queue>

#include <glad/glad.h>

#include <final_platform_layer.h>

#include <assert.h>

namespace renderer {

	struct OpenGLBuffer: public Buffer {
		GLuint bindId;
		OpenGLBuffer(const BufferID id, const BufferType type, const size_t size, const BufferAccess access, const BufferUsage usage):
			Buffer(id, type, size, access, usage),
			bindId(0) {
		}
	};

	struct OpenGLTexture: public Texture {
		GLuint nativeId;
		GLenum nativeTarget;
		GLenum nativeFormat;
		GLenum nativeType;
		GLint nativeInternalFormat;

		OpenGLTexture(const TextureID id, const TextureType type, const TextureFormat format, const uint32_t width, const uint32_t height, const uint32_t depth, const uint8_t *data):
			Texture(id, type, format, width, height, depth),
			nativeId(0),
			nativeTarget(0),
			nativeFormat(0),
			nativeType(0),
			nativeInternalFormat(0) {

			switch(type) {
				case TextureType::T2D:
					nativeTarget = GL_TEXTURE_2D;
					break;
				case TextureType::Cube:
					nativeTarget = GL_TEXTURE_CUBE_MAP;
					break;
			}

			uint32_t bytesPerPixel = 0;
			switch(format) {
				case TextureFormat::AlphaU8:
					nativeInternalFormat = GL_ALPHA8;
					nativeFormat = GL_ALPHA;
					bytesPerPixel = 8;
					break;

				case TextureFormat::RGBAU8:
					nativeInternalFormat = GL_RGBA8;
					nativeFormat = GL_RGBA;
					bytesPerPixel = sizeof(uint8_t) * 4;
					break;

				case TextureFormat::RGBAF32:
					nativeInternalFormat = GL_RGB32F;
					nativeFormat = GL_RGBA;
					bytesPerPixel = sizeof(float) * 4;
					break;

				default:
					assert(!"Unsupported texture format!");
					break;
			}

			glGenTextures(1, &nativeId);
			glBindTexture(nativeTarget, nativeId);
			switch(type) {
				case TextureType::T2D:
					glTexImage2D(nativeTarget, 0, nativeInternalFormat, width, height, 0, nativeFormat, nativeType, static_cast<const void *>(data));
					break;

				case TextureType::Cube:
				{
					size_t faceSize = (size_t)width * (size_t)height * (size_t)bytesPerPixel;
					for(uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
						const uint8_t *facePixels = data + (faceSize * faceIndex);
						GLuint texTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex;
						glTexImage2D(texTarget, 0, nativeInternalFormat, width, height, 0, nativeFormat, nativeType, static_cast<const void *>(facePixels));
					}
				} break;

				default:
					assert(!"Unsupported texture type!");
					break;
			}
			glBindTexture(nativeTarget, 0);
		}

		~OpenGLTexture() {
			if(nativeId > 0) {
				glDeleteTextures(1, &nativeId);
			}
		}
	};

	enum class CommandType {
		None = 0,
		SetViewport,
		SetPipeline,
		SetBuffer,
		Draw,
	};

	struct alignas(16) CommandHeader {
		CommandType type;
		size_t size;
	};

	struct ViewportCommand {
		int32_t x;
		int32_t y;
		int32_t width;
		int32_t height;
	};

	struct DrawCommand {
		size_t vertexCount;
		size_t firstVertex;
		size_t instanceCount;
		size_t firstInstance;
	};

	struct SetPipelineCommand {
		PipelineID pipeline;
	};

	struct SetBufferCommand {
		BufferID buffer;
	};

	struct CommandBufferChunk {
		constexpr static size_t Alignment = 16;
		constexpr static size_t MinChunkSize = 4096;
		uint8_t *data;
		size_t capacity;
		size_t offset;
		size_t used;

		CommandBufferChunk(const size_t size = 0):
			data(nullptr),
			capacity(0),
			offset(0),
			used(0) {
			capacity = std::max(MinChunkSize, size);
			data = new uint8_t[capacity];
			offset = 0;
			used = 0;
		}

		bool DoesFit(const size_t size) {
			size_t requiredSize = sizeof(CommandHeader) + size;
			bool result = (used + requiredSize) <= capacity;
			return(result);
		}

		void Push(const CommandType type, const size_t commandSize, const uint8_t *commandData) {
			size_t requiredSize = sizeof(CommandHeader) + commandSize;
			assert(requiredSize > 0 && (offset + requiredSize) <= capacity);
			assert(commandData != nullptr);
			size_t start = offset;
			uint8_t *target = data + start;
			CommandHeader header = {};
			header.type = type;
			header.size = commandSize;
			std::memcpy(target + 0, &header, sizeof(CommandHeader));
			std::memcpy(target + sizeof(CommandHeader), commandData, commandSize);
			offset += requiredSize;
			used += requiredSize;
		}

		~CommandBufferChunk() {
			delete[] data;
		}
	};

	class DefaultCommandBuffer: public CommandBuffer {
	private:
		enum class CommandBufferRecordingState: int32_t {
			Ready = 0,
			Recording,
			WaitingForSubmit,
		};

		Renderer *renderer;
		CommandBufferRecordingState state;
		std::queue<CommandBufferChunk *> chunks;

		void Clear() {
			// In case the command buffer was never submitted, just wipe it
			while(!chunks.empty()) {
				CommandBufferChunk *chunk = chunks.front();

				// TODO(final): Memory arena (No delete required)
				delete chunk;

				chunks.pop();
			}
		}
	protected:
		void Push(const CommandType type, const size_t size, const uint8_t *data) {
			assert(type != CommandType::None && size > 0 && data != nullptr);

			if(chunks.empty()) {
				// TODO(final): Memory arena (Push)
				CommandBufferChunk *newChunk = new CommandBufferChunk(size);
				chunks.push(newChunk);
			}

			CommandBufferChunk *chunk = chunks.back();
			assert(chunk != nullptr);
			if(!chunk->DoesFit(size)) {
				// TODO(final): Memory arena (Push)
				CommandBufferChunk *newChunk = chunk = new CommandBufferChunk(size);
				chunks.push(newChunk);
			}

			assert(chunk != nullptr);

			chunk->Push(type, size, data);
		}
	public:
		CommandBufferChunk *Pop() {
			if(state != CommandBufferRecordingState::WaitingForSubmit) return(nullptr);
			if(chunks.empty()) return(nullptr);
			CommandBufferChunk *chunk = chunks.front();
			if(chunks.empty()) {
				// We have cleared the chunks queue, we can now accept new stuff
				state = CommandBufferRecordingState::Ready;
			}
			return(chunk);
		}

		DefaultCommandBuffer(Renderer *renderer):
			renderer(renderer),
			state(CommandBufferRecordingState::Ready) {
		}

		bool Begin() override {
			if(state != CommandBufferRecordingState::Ready) return(false);
			state = CommandBufferRecordingState::Recording;
			Clear();
			return(true);
		}

		void End() override {
			if(state != CommandBufferRecordingState::Recording) return;
			state = CommandBufferRecordingState::WaitingForSubmit;
		}

		void SetViewport(const int x, const int y, const int width, const int height) override {
			if(state != CommandBufferRecordingState::Recording) return;
			ViewportCommand cmd = {};
			cmd.x = x;
			cmd.y = y;
			cmd.width = width;
			cmd.height = height;
			const uint8_t *data = (const uint8_t *)&cmd;
			Push(CommandType::SetViewport, sizeof(cmd), data);
		}

		void SetPipeline(const Pipeline &pipeline) override {
			if(state != CommandBufferRecordingState::Recording) return;
			const uint8_t *data = (const uint8_t *)&pipeline;
			Push(CommandType::SetPipeline, sizeof(pipeline), data);
		}

		void Draw(const size_t vertexCount, const size_t firstVertex, const size_t instanceCount, const size_t firstInstance) override {
			if(state != CommandBufferRecordingState::Recording) return;
			DrawCommand cmd = {};
			cmd.vertexCount = vertexCount;
			cmd.firstVertex = firstVertex;
			cmd.instanceCount = instanceCount;
			cmd.firstInstance = firstInstance;
			const uint8_t *data = (const uint8_t *)&cmd;
			Push(CommandType::Draw, sizeof(cmd), data);
		}


		void SetBuffer(const Buffer &buffer) override {
			if(state != CommandBufferRecordingState::Recording) return;
		}

	};

	class BaseRenderer: public Renderer {
	private:
		volatile uint32_t _idCounter;
		std::map<BufferID, Buffer *> _buffersMap;
		std::map<TextureID, Texture *> _texturesMap;
		std::vector<CommandBuffer *> _commandBuffers;
	protected:
		~BaseRenderer() {
			for(auto commandBuffer : _commandBuffers) {
				delete commandBuffer;
			}
			for(auto texturePair : _texturesMap) {
				Texture *texture = texturePair.second;
				delete texture;
			}
			for(auto bufferPair : _buffersMap) {
				Buffer *buffer = bufferPair.second;
				delete buffer;
			}
		}

		inline uint32_t NextID() {
			uint32_t result = fplAtomicIncrementU32(&_idCounter);
			return(result);
		}

		void AddBuffer(Buffer &buffer) {
			_buffersMap.insert(std::pair<BufferID, Buffer *>(buffer.id, &buffer));
		}
		void RemoveBuffer(const BufferID id) {
			_buffersMap.erase(id);
		}
		void AddTexture(Texture &texture) {
			_texturesMap.insert(std::pair<TextureID, Texture *>(texture.id, &texture));
		}
		void RemoveTexture(const TextureID id) {
			_texturesMap.erase(id);
		}
		void AddCommandBuffer(CommandBuffer &commandBuffer) {
			_commandBuffers.push_back(&commandBuffer);
		}
		void RemoveCommandBuffer(CommandBuffer &commandBuffer) {
			_commandBuffers.erase(std::remove(_commandBuffers.begin(), _commandBuffers.end(), &commandBuffer));
		}
	};

	class OpenGLRenderer;

	class OpenGLCommandBuffer: public DefaultCommandBuffer {
	public:
		OpenGLCommandBuffer(OpenGLRenderer *renderer):
			DefaultCommandBuffer((Renderer *)renderer) {
		}
	};

	class OpenGLCommandQueue: public CommandQueue {
	private:
		OpenGLRenderer *renderer;
	public:
		OpenGLCommandQueue(OpenGLRenderer *renderer):
			CommandQueue(),
			renderer(renderer) {
		}

		void ExecuteCommand(const CommandType type, const uint8_t *data, const size_t size) {
			switch(type) {
				case CommandType::SetViewport:
				{
					const ViewportCommand *vpCmd = (const ViewportCommand *)data;
					glViewport(vpCmd->x, vpCmd->y, vpCmd->width, vpCmd->height);
				} break;

				default:
					break;
			}
		}

		bool Submit(CommandBuffer &commandBuffer) override {
			OpenGLCommandBuffer *nativeCommandBuffer = static_cast<OpenGLCommandBuffer *>(&commandBuffer);
			CommandBufferChunk *chunk;
			while((chunk = nativeCommandBuffer->Pop()) != nullptr) {
				size_t offset = 0;
				size_t remaining = chunk->used;
				const uint8_t *start = chunk->data;
				while(remaining >= sizeof(CommandHeader)) {
					const CommandHeader *header = (const CommandHeader *)(start + offset);

					// TODO(final): Validate header!
					CommandType cmdType = header->type;
					size_t dataSize = header->size;

					start += sizeof(header);
					remaining -= sizeof(CommandHeader);

					if(dataSize > 0) {
						ExecuteCommand(cmdType, start, dataSize);
						remaining -= dataSize;
					}
				}
			}
			return(true);
		}

	};

	class OpenGLRenderer: public BaseRenderer {
	private:
		OpenGLCommandQueue *_commandQueue;
	public:
		OpenGLRenderer(): BaseRenderer() {
			_commandQueue = new	OpenGLCommandQueue(this);
		}

		~OpenGLRenderer() {
			delete _commandQueue;
		}

		Buffer *CreateBuffer(const BufferType type, const size_t size, const BufferAccess access, const BufferUsage usage) override {
			BufferID id = { NextID() };
			OpenGLBuffer *newBuffer = new OpenGLBuffer(id, type, size, access, usage);
			AddBuffer(*newBuffer);
			return(newBuffer);
		}

		void DestroyBuffer(Buffer &buffer) override {
			OpenGLBuffer *nativeBuffer = reinterpret_cast<OpenGLBuffer *>(&buffer);
			RemoveBuffer(nativeBuffer->id);
			delete nativeBuffer;
		}

		Texture *CreateTexture2D(const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data2D) override {
			TextureID id = { NextID() };
			OpenGLTexture *newTexture = new OpenGLTexture(id, TextureType::T2D, format, width, height, 0, data2D);
			AddTexture(*newTexture);
			return(newTexture);
		}


		Texture *CreateTextureCube(const TextureFormat format, const uint32_t faceWidth, const uint32_t faceHeight, const uint8_t *data2Dx6) override {
			TextureID id = { NextID() };
			OpenGLTexture *newTexture = new OpenGLTexture(id, TextureType::Cube, format, faceWidth, faceHeight, 0, data2Dx6);
			AddTexture(*newTexture);
			return(newTexture);
		}

		void DestroyTexture(Texture &texture) override {
			OpenGLTexture *nativeTexture = reinterpret_cast<OpenGLTexture *>(&texture);
			RemoveTexture(nativeTexture->id);
			delete nativeTexture;
		}

		CommandQueue *GetCommandQueue() override {
			return _commandQueue;
		}


		CommandBuffer *CreateCommandBuffer() override {
			OpenGLCommandBuffer *commandBuffer = new OpenGLCommandBuffer(this);
			AddCommandBuffer(*commandBuffer);
			return(commandBuffer);
		}


		void DestroyCommandBuffer(CommandBuffer &commandBuffer) override {
			OpenGLCommandBuffer *nativeCommandBuffer = static_cast<OpenGLCommandBuffer *>(&commandBuffer);
			RemoveCommandBuffer(*nativeCommandBuffer);
			delete nativeCommandBuffer;
		}


		void Present() override {
		}

	};

	Renderer *Renderer::Create(const RendererType type) {
		switch(type) {
			case RendererType::OpenGL:
				return new OpenGLRenderer();
			default:
				assert(!"Renderer type not supported!");
				break;
		}
		return(nullptr);
	}

};