#include "Renderer2.h"

#include <map>
#include <queue>
#include <stdexcept>
#include <assert.h>

#include <glad/glad.h>

#include <final_platform_layer.h>

namespace fsr {

	struct OpenGLBuffer: public Buffer {
		GLuint nativeId;
		GLenum nativeTarget;
		GLenum nativeUsage;
		GLenum nativeAccess;

		OpenGLBuffer(const BufferID id, const BufferType type, const BufferAccess access, const BufferUsage usage, const size_t size):
			Buffer(id, type, access, usage, size),
			nativeId(0),
			nativeTarget(0),
			nativeUsage(0),
			nativeAccess(0) {
		}

		void Write(const size_t offset, const size_t size, const uint8_t *data) override {
			glBindBuffer(nativeTarget, nativeId);
			glBufferSubData(nativeTarget, (GLintptr)offset, size, data);
			glBindBuffer(nativeTarget, 0);
		}

		void *Map() {


			glBindBuffer(nativeTarget, nativeId);
			void *result = glMapBuffer(nativeTarget, nativeAccess);
			return(result);
		}

		void Unmap() override {
			glUnmapBuffer(nativeTarget);
			glBindBuffer(nativeTarget, 0);
		}

		~OpenGLBuffer() {

		}

		bool Init(const uint8_t *data) override {
			switch(type) {
				case BufferType::Vertex:
					nativeTarget = GL_ARRAY_BUFFER;
					break;

				case BufferType::Index:
					nativeTarget = GL_ELEMENT_ARRAY_BUFFER;
					break;

				case BufferType::Uniform:
					nativeTarget = GL_UNIFORM_BUFFER;
					break;

				default:
					assert(!"Not supported buffer type");
					nativeTarget = 0;
					return(false);
			}

			switch(access) {
				case BufferAccess::ReadWrite:
					nativeAccess = GL_READ_WRITE;
					break;

				case BufferAccess::ReadOnly:
					nativeAccess = GL_READ_ONLY;
					break;

				case BufferAccess::WriteOnly:
					nativeAccess = GL_WRITE_ONLY;
					break;

				default:
					nativeAccess = 0;
					assert(!"Not supported buffer access");
					return(false);
			}

			switch(usage) {
				case BufferUsage::Static:
					nativeUsage = GL_STATIC_DRAW;
					break;

				case BufferUsage::Dynamic:
					nativeUsage = GL_DYNAMIC_DRAW;
					break;

				default:
					nativeUsage = 0;
					assert(!"Not supported buffer usage");
					return(false);
			}

			glGenBuffers(1, &nativeId);
			if(nativeId == 0) {
				return(false);
			}

			glBindBuffer(nativeTarget, nativeId);
			glBufferData(nativeTarget, size, data, nativeUsage);
			glBindBuffer(nativeTarget, 0);
			return(true);
		}


		void Release() override {
			if(nativeId > 0) {
				glDeleteBuffers(1, &nativeId);
			}
		}

	};

	struct OpenGLTexture: public Texture {
	private:
		GLuint nativeId;
		GLenum nativeTarget;
		GLenum nativeFormat;
		GLenum nativeType;
		GLint nativeInternalFormat;
		uint32_t bytesPerPixel;
	public:
		OpenGLTexture(const TextureID id, const TextureType type, const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data):
			Texture(id, type, format, width, height),
			nativeId(0),
			nativeTarget(0),
			nativeFormat(0),
			nativeType(0),
			nativeInternalFormat(0),
			bytesPerPixel(0) {

			switch(type) {
				case TextureType::T2D:
					nativeTarget = GL_TEXTURE_2D;
					break;
				case TextureType::Cube:
					nativeTarget = GL_TEXTURE_CUBE_MAP;
					break;
			}

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
				{
					glTexImage2D(nativeTarget, 0, nativeInternalFormat, width, height, 0, nativeFormat, nativeType, static_cast<const void *>(data));
				} break;

				case TextureType::Cube:
				{
					size_t faceSize = (size_t)width * (size_t)height * (size_t)bytesPerPixel;
					for(uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
						const uint8_t *facePixels;
						if(data != nullptr)
							facePixels = data + (faceSize * faceIndex);
						else
							facePixels = nullptr;
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

		bool Write(const size_t size, const uint8_t *data) override {
			size_t requiredSize = (size_t)(width * height * bytesPerPixel);
			if(size != requiredSize || data == nullptr) {
				return(false);
			}
			glBindTexture(nativeTarget, nativeId);
			switch(type) {
				case TextureType::T2D:
				{
					glTexImage2D(nativeTarget, 0, nativeInternalFormat, width, height, 0, nativeFormat, nativeType, static_cast<const void *>(data));
				} break;

				case TextureType::Cube:
				{
					size_t faceSize = (size_t)width * (size_t)height * (size_t)bytesPerPixel;
					for(uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex) {
						const uint8_t *facePixels;
						if(data != nullptr)
							facePixels = data + (faceSize * faceIndex);
						else
							facePixels = nullptr;
						GLuint texTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIndex;
						glTexImage2D(texTarget, 0, nativeInternalFormat, width, height, 0, nativeFormat, nativeType, static_cast<const void *>(facePixels));
					}
				} break;

				default:
					assert(!"Unsupported texture type!");
					break;
			}
			glBindTexture(nativeTarget, 0);
			return(true);
		}
	};

	struct OpenGLRenderTarget: public FrameBuffer {
	private:
		GLuint nativeId;
	public:
		OpenGLRenderTarget(const FrameBufferID &id, const uint32_t sampleCount):
			FrameBuffer(id, sampleCount),
			nativeId(0) {
		}

		bool Init(const std::initializer_list<FrameBufferAttachment> &attachments) override {
			if(attachments.size() == 0) return(false); // No attachment
			glGenFramebuffers(1, &nativeId);
			if(nativeId == 0) return(false);
			return(true);
		}

		void Release() override {
			if(nativeId > 0) {
				glDeleteFramebuffers(1, &nativeId);
				nativeId = 0;
			}
		}
	};

	enum class CommandType {
		None = 0,
		BindPipeline,
		SetViewport,
		SetScissor,
		BindVertexBuffers,
		BindIndexBuffers,
		BeginRenderPass,
		EndRenderPass,
		Draw,
	};

	struct alignas(16) CommandHeader {
		CommandType type;
		size_t size;
	};

	struct ClearCommand {
		ClearFlags flags;
	};

	struct BindPipelineCommand {
		PipelineID pipelineId;
	};

	struct SetViewportCommand {
		Viewport viewportRect;
	};

	struct SetScissorCommand {
		ScissorRect scissorRect;
	};

	struct BindBuffersCommand {
		BufferType bufferType;
		size_t bufferCount;
		// The data are a array of BufferIDs
	};

	struct BeginRenderPassCommand {
		constexpr static uint32_t MaxClearValueCount = 8;
		RenderArea renderArea;
		RenderPassID renderPassId;
		FrameBufferID renderTargetId;
		ClearValue clearValues[MaxClearValueCount];
		uint32_t clearValueCount;
	};

	struct DrawCommand {
		size_t vertexCount;
		size_t firstVertex;
		size_t instanceCount;
		size_t firstInstance;
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

		CommandHeader *PushHeader(const CommandType type, const size_t commandSize) {
			size_t requiredSize = sizeof(CommandHeader) + commandSize;
			assert(requiredSize > 0 && (offset + requiredSize) <= capacity);
			size_t start = offset;
			uint8_t *target = data + start;
			CommandHeader header = {};
			header.type = type;
			header.size = commandSize;
			std::memcpy(target + 0, &header, sizeof(CommandHeader));
			offset += requiredSize;
			used += requiredSize;
			CommandHeader *result = reinterpret_cast<CommandHeader *>(target);
			return(result);
		}

		void PushData(CommandHeader *header, const size_t offset, const size_t commandSize, const uint8_t *commandData) {
			uint8_t *target = (uint8_t *)header + sizeof(CommandHeader) + offset;
			std::memcpy(target, commandData, commandSize);
		}

		void Push(const CommandType type, const size_t commandSize, const uint8_t *commandData) {
			CommandHeader *header = PushHeader(type, commandSize);
			assert(header != nullptr);
			PushData(header, 0, commandSize, commandData);
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
		void Push(const CommandType type, const size_t size, const uint8_t *data, const size_t additionalDataSize = 0, const uint8_t *additionalData = nullptr) {
			assert(type != CommandType::None && size > 0 && data != nullptr);

			size_t requiredSize = size;
			if(additionalDataSize > 0 && additionalData != fpl_null) {
				requiredSize += additionalDataSize;
			}

			if(chunks.empty()) {
				// TODO(final): Memory arena (Push)
				CommandBufferChunk *newChunk = new CommandBufferChunk(requiredSize);
				chunks.push(newChunk);
			}

			CommandBufferChunk *chunk = chunks.back();
			assert(chunk != nullptr);
			if(!chunk->DoesFit(requiredSize)) {
				// TODO(final): Memory arena (Push)
				CommandBufferChunk *newChunk = chunk = new CommandBufferChunk(requiredSize);
				chunks.push(newChunk);
			}

			assert(chunk != nullptr);

			if(additionalDataSize > 0 && additionalData != fpl_null) {
				CommandHeader *header = chunk->PushHeader(type, requiredSize);
				chunk->PushData(header, 0, size, data);
				chunk->PushData(header, size, additionalDataSize, additionalData);
			} else {
				chunk->Push(type, requiredSize, data);
			}
		}
	public:
		CommandBufferChunk *Pop() {
			if(state != CommandBufferRecordingState::WaitingForSubmit) return(nullptr);
			if(chunks.empty()) return(nullptr);
			CommandBufferChunk *chunk = chunks.front();
			chunks.pop();
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

		void SetViewport(const float x, const float y, const float width, const float height, const float minDepth, const float maxDepth) override {
			if(state != CommandBufferRecordingState::Recording) return;
			SetViewportCommand cmd = {};
			cmd.viewportRect = Viewport(x, y, width, height, minDepth, maxDepth);
			Push(CommandType::SetViewport, sizeof(cmd), (const uint8_t *)&cmd);
		}

		void SetScissor(const int x, const int y, const int width, const int height) override {
			if(state != CommandBufferRecordingState::Recording) return;
			SetScissorCommand cmd = {};
			cmd.scissorRect = ScissorRect(x, y, width, height);
			Push(CommandType::SetViewport, sizeof(cmd), (const uint8_t *)&cmd);
		}

		void BindPipeline(const PipelineID &pipelineId) override {
			if(state != CommandBufferRecordingState::Recording) return;
			BindPipelineCommand cmd = {};
			cmd.pipelineId = pipelineId;
			Push(CommandType::BindPipeline, sizeof(cmd), (const uint8_t *)&cmd);
		}

		void BindVertexBuffers(const std::initializer_list<const BufferID> &ids) override {
			if(state != CommandBufferRecordingState::Recording || ids.size() == 0) return;
			BindBuffersCommand cmd = {};
			cmd.bufferType = BufferType::Vertex;
			cmd.bufferCount = ids.size();
			size_t dataSize = sizeof(BufferID) * cmd.bufferCount;
			const uint8_t *data = (const uint8_t *)ids.begin();
			Push(CommandType::BindVertexBuffers, sizeof(cmd), (const uint8_t *)&cmd, dataSize, data);
		}

		void BindIndexBuffers(const std::initializer_list<const BufferID> &ids) override {
			if(state != CommandBufferRecordingState::Recording || ids.size() == 0) return;
			BindBuffersCommand cmd = {};
			cmd.bufferType = BufferType::Index;
			cmd.bufferCount = ids.size();
			size_t dataSize = sizeof(BufferID) * cmd.bufferCount;
			const uint8_t *data = (const uint8_t *)ids.begin();
			Push(CommandType::BindIndexBuffers, sizeof(cmd), (const uint8_t *)&cmd, dataSize, data);
		}

		void BeginRenderPass(const RenderPassID &renderPassId, const FrameBufferID &frameBufferId, const RenderArea *renderArea, const std::initializer_list<const ClearValue> &clearValues) {
			if(state != CommandBufferRecordingState::Recording) return;
		}

		void EndRenderPass() {
			if(state != CommandBufferRecordingState::Recording) return;
		}

		void Draw(const size_t vertexCount, const size_t firstVertex, const size_t instanceCount, const size_t firstInstance) override {
			if(state != CommandBufferRecordingState::Recording) return;
			DrawCommand cmd = {};
			cmd.vertexCount = vertexCount;
			cmd.firstVertex = firstVertex;
			cmd.instanceCount = instanceCount;
			cmd.firstInstance = firstInstance;
			Push(CommandType::Draw, sizeof(cmd), (const uint8_t *)&cmd);
		}
	};

	class BaseRenderer: public Renderer {
	private:
		volatile uint32_t _idCounter;
		std::map<BufferID, Buffer *> _buffersMap;
		std::map<TextureID, Texture *> _texturesMap;
		std::map<FrameBufferID, FrameBuffer *> _renderTargetMap;
		std::map<PipelineID, Pipeline *> _pipelineMap;
		std::vector<CommandBuffer *> _commandBuffers;
	protected:
		BaseRenderer(): 
			_idCounter(0) {

		}

		~BaseRenderer() {
			Release();
		}

		virtual bool Init() override {
			_idCounter = 0;
			return(true);
		}

		virtual void Release() override {
			for(auto commandBuffer : _commandBuffers) {
				delete commandBuffer;
			}
			for(auto pipelinePair : _pipelineMap) {
				Pipeline *pipeline = pipelinePair.second;
				delete pipeline;
			}
			for(auto renderTargetPair : _renderTargetMap) {
				FrameBuffer *renderTarget = renderTargetPair.second;
				delete renderTarget;
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

		void AddBuffer(Buffer *buffer) {
			assert(buffer != nullptr);
			_buffersMap.insert(std::pair<BufferID, Buffer *>(buffer->id, buffer));
		}
		Buffer *RemoveBuffer(const BufferID bufferId) {
			std::map<BufferID, Buffer *>::iterator found = _buffersMap.find(bufferId);
			if(found != _buffersMap.end()) {
				Buffer *buffer = found->second;
				_buffersMap.erase(bufferId);
				return(buffer);
			}
			return(nullptr);
		}

		void AddTexture(Texture *texture) {
			assert(texture != nullptr);
			_texturesMap.insert(std::pair<TextureID, Texture *>(texture->id, texture));
		}
		Texture *RemoveTexture(const TextureID textureId) {
			std::map<TextureID, Texture *>::iterator found = _texturesMap.find(textureId);
			if(found != _texturesMap.end()) {
				Texture *texture = found->second;
				_texturesMap.erase(textureId);
				return(texture);
			}
			return(nullptr);
		}

		void AddRenderTarget(FrameBuffer *renderTarget) {
			assert(renderTarget != nullptr);
			_renderTargetMap.insert(std::pair<FrameBufferID, FrameBuffer *>(renderTarget->id, renderTarget));
		}
		FrameBuffer *RemoveRenderTarget(const FrameBufferID renderTargetId) {
			std::map<FrameBufferID, FrameBuffer *>::iterator found = _renderTargetMap.find(renderTargetId);
			if(found != _renderTargetMap.end()) {
				FrameBuffer *renderTarget = found->second;
				_renderTargetMap.erase(renderTargetId);
				return(renderTarget);
			}
			return(nullptr);
		}

		void AddPipeline(Pipeline *pipeline) {
			assert(pipeline != nullptr);
			_pipelineMap.insert(std::pair<PipelineID, Pipeline *>(pipeline->id, pipeline));
		}
		Pipeline *RemovePipeline(const PipelineID pipelineId) {
			std::map<PipelineID, Pipeline *>::iterator found = _pipelineMap.find(pipelineId);
			if(found != _pipelineMap.end()) {
				Pipeline *pipeline = found->second;
				_pipelineMap.erase(pipelineId);
				return(pipeline);
			}
			return(nullptr);
		}

		void AddCommandBuffer(CommandBuffer *commandBuffer) {
			assert(commandBuffer != nullptr);
			_commandBuffers.push_back(commandBuffer);
		}
		void RemoveCommandBuffer(CommandBuffer *commandBuffer) {
			assert(commandBuffer != nullptr);
			_commandBuffers.erase(std::remove(_commandBuffers.begin(), _commandBuffers.end(), commandBuffer));
		}
	public:
		Buffer *GetBuffer(const BufferID bufferId) {
			Buffer *result = _buffersMap[bufferId];
			return(result);
		}

		Pipeline *GetPipeline(const PipelineID pipelineId) {
			Pipeline *result = _pipelineMap[pipelineId];
			return(result);
		}

		Texture *GetTexture(const TextureID textureID) {
			Texture *result = _texturesMap[textureID];
			return(result);
		}

		FrameBuffer *GetRenderTarget(const FrameBufferID renderTargetId) {
			FrameBuffer *result = _renderTargetMap[renderTargetId];
			return(result);
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

		struct PipelineSubmitState {
			Pipeline *activePipeline;
		};

		void ChangePipeline(PipelineSubmitState *submitState, const PipelineID pipelineId) {
			BaseRenderer *baseRenderer = reinterpret_cast<BaseRenderer *>(renderer);

			Pipeline *pipeline = baseRenderer->GetPipeline(pipelineId);
			assert(pipeline != nullptr);
			submitState->activePipeline = pipeline;

			Viewport viewport = pipeline->viewport;
			ScissorRect scissor = pipeline->scissor;

			ClearValue clearValue = pipeline->settings.clear.value;
			glm::vec4 clearColor = clearValue.color.v4;
			float clearDepth = clearValue.depthStencil.depth;
			int clearStencil = clearValue.depthStencil.stencil;

			glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
			glClearDepth(clearDepth);
			glClearStencil(clearStencil);

			glDepthRange(viewport.minDepth, viewport.maxDepth);

			glViewport((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
			glScissor(scissor.x, scissor.y, scissor.width, scissor.height);
		}

		void ExecuteCommand(PipelineSubmitState *submitState, const CommandType type, const uint8_t *data, const size_t size) {
			switch(type) {
				case CommandType::SetViewport:
				{
					const SetViewportCommand *cmd = (const SetViewportCommand *)data;
					Viewport viewport = cmd->viewportRect;
					glViewport((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
				} break;

				case CommandType::SetScissor:
				{
					const SetScissorCommand *cmd = (const SetScissorCommand *)data;
					ScissorRect scissor = cmd->scissorRect;
					glScissor(scissor.x, scissor.y, scissor.width, scissor.height);
				} break;

				case CommandType::BindPipeline:
				{
					const BindPipelineCommand *cmd = (const BindPipelineCommand *)data;
					PipelineID pipelineId = cmd->pipelineId;
					ChangePipeline(submitState, pipelineId);
				} break;

				default:
					assert(!"Unsupported command type!");
					break;
			}
		}
	public:
		OpenGLCommandQueue(OpenGLRenderer *renderer):
			CommandQueue(),
			renderer(renderer) {
		}

		bool Submit(CommandBuffer &commandBuffer) override {
			PipelineSubmitState submitState = {};
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

					remaining -= sizeof(CommandHeader);
					offset += sizeof(CommandHeader);

					if(dataSize > 0) {
						const uint8_t *dataStart = (const uint8_t *)header + sizeof(CommandHeader);
						ExecuteCommand(&submitState, cmdType, dataStart, dataSize);
						remaining -= dataSize;
						offset += dataSize;
					}
				}
				int x = 0;
			}
			return(true);
		}

	};

	class OpenGLRenderer: public BaseRenderer {
	private:
		OpenGLCommandQueue *_commandQueue;

		void SetDefault() {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
			glDepthMask(GL_TRUE);
			glDepthRange(0.0f, 1.0f);

			glClearDepth(1.0f);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			glFrontFace(GL_CCW);
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glDisable(GL_TEXTURE_3D);
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_1D);
			glDisable(GL_TEXTURE_CUBE_MAP);

			glShadeModel(GL_SMOOTH);

			glLineWidth(1.0f);
			glPointSize(1.0f);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glMatrixMode(GL_MODELVIEW);
		}
	public:
		OpenGLRenderer(): BaseRenderer() {
			_commandQueue = new	OpenGLCommandQueue(this);
			SetDefault();
		}

		~OpenGLRenderer() {
			delete _commandQueue;
		}

		BufferID CreateBuffer(const BufferType type, const BufferAccess access, const BufferUsage usage, const size_t size, const uint8_t *data) override {
			BufferID id = BufferID { NextID() };
			OpenGLBuffer *newBuffer = new OpenGLBuffer(id, type, access, usage, size);
			if(!newBuffer->Init(data)) {
				delete newBuffer;
				return(BufferID { 0 });
			}
			AddBuffer(newBuffer);
			return(id);
		}

		void DestroyBuffer(const BufferID bufferId) override {
			Buffer *buffer = RemoveBuffer(bufferId);
			if(buffer != nullptr) {
				buffer->Release();
				delete buffer;
			}
		}

		TextureID CreateTexture2D(const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data2D) override {
			TextureID id = TextureID { NextID() };
			OpenGLTexture *newTexture = new OpenGLTexture(id, TextureType::T2D, format, width, height, data2D);
			AddTexture(newTexture);
			return(id);
		}

		TextureID CreateTextureCube(const TextureFormat format, const uint32_t faceWidth, const uint32_t faceHeight, const uint8_t *data2Dx6) override {
			TextureID id = TextureID { NextID() };
			OpenGLTexture *newTexture = new OpenGLTexture(id, TextureType::Cube, format, faceWidth, faceHeight, data2Dx6);
			AddTexture(newTexture);
			return(id);
		}

		void DestroyTexture(const TextureID textureId) override {
			Texture *texture = RemoveTexture(textureId);
			if(texture != nullptr) {
				delete texture;
			}
		}

		CommandQueue *GetCommandQueue() override {
			return _commandQueue;
		}


		CommandBuffer *CreateCommandBuffer() override {
			OpenGLCommandBuffer *commandBuffer = new OpenGLCommandBuffer(this);
			AddCommandBuffer(commandBuffer);
			return(commandBuffer);
		}


		void DestroyCommandBuffer(CommandBuffer *commandBuffer) override {
			if(commandBuffer == nullptr) return;
			RemoveCommandBuffer(commandBuffer);
			delete commandBuffer;
		}

		FrameBufferID CreateFrameBuffer(const std::initializer_list<FrameBufferAttachment> &attachments, const uint32_t sampleCount) override {
			FrameBufferID id = FrameBufferID { NextID() };
			OpenGLRenderTarget *renderTarget = new OpenGLRenderTarget(id, sampleCount);
			if(!renderTarget->Init(attachments)) {
				delete renderTarget;
				return(FrameBufferID { 0 });
			}
			AddRenderTarget(renderTarget);
			return(id);
		}


		void DestroyFrameBuffer(const FrameBufferID renderTargetId) override {
			FrameBuffer *renderTarget = RemoveRenderTarget(renderTargetId);
			if(renderTarget != nullptr) {
				renderTarget->Release();
				delete renderTarget;
			}
		}

		PipelineID CreatePipeline(const PipelineDescriptor &pipelineDesc) override {
			PipelineID id = PipelineID { NextID() };
			Pipeline *pipeline = new Pipeline(id);
			pipeline->layoutId = pipelineDesc.layoutId;
			pipeline->settings = pipelineDesc.settings;
			pipeline->primitive = pipelineDesc.primitive;
			pipeline->frameBufferId = pipelineDesc.frameBuffertId;
			pipeline->scissor = pipelineDesc.scissor;
			pipeline->viewport = pipelineDesc.viewport;
			pipeline->shaderProgramId = pipelineDesc.shaderProgramId;
			AddPipeline(pipeline);
			return(id);
		}

		void DestroyPipeline(const PipelineID pipelineId) override {
			Pipeline *pipeline = RemovePipeline(pipelineId);
			if(pipeline != nullptr) {
				delete pipeline;
			}
		}

		void Present() override {
			fplVideoFlip();
		}
	};

	Renderer *Renderer::Create(const RendererType type) {
		Renderer *result = nullptr;
		switch(type) {
			case RendererType::OpenGL:
				result = new OpenGLRenderer();
				break;

			default:
				assert(!"Renderer type not supported!");
				break;
		}
		if(result != nullptr && !result->Init()) {
			delete result;
			return(nullptr);
		}
		return(result);
	}

};