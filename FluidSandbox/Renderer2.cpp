#include "Renderer2.h"

#include <map>

#include <glad/glad.h>

#include <final_platform_layer.h>

#include <assert.h>

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
			case TextureType::T1D:
				nativeTarget = GL_TEXTURE_1D;
				break;
			case TextureType::T2D:
				nativeTarget = GL_TEXTURE_2D;
				break;
			case TextureType::T3D:
				nativeTarget = GL_TEXTURE_3D;
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

class BaseRenderer: public Renderer {
private:
	volatile uint32_t _idCounter;
protected:
	inline uint32_t NextID() {
		uint32_t result = fplAtomicIncrementU32(&_idCounter);
		return(result);
	}
	std::map<BufferID, Buffer *> _buffersMap;
	std::map<TextureID, Texture *> _texturesMap;
};

class OpenGLRenderer: public BaseRenderer {
public:
	Buffer *CreateBuffer(const BufferType type, const size_t size, const BufferAccess access, const BufferUsage usage) override {
		BufferID id = { NextID() };
		OpenGLBuffer *newBuffer = new OpenGLBuffer(id, type, size, access, usage);
		_buffersMap.insert(std::pair<BufferID, Buffer *>(id, newBuffer));
		return(newBuffer);
	}

	void DestroyBuffer(Buffer &buffer) override {
		OpenGLBuffer *nativeBuffer = reinterpret_cast<OpenGLBuffer *>(&buffer);
		_buffersMap.erase(buffer.id);
		delete nativeBuffer;
	}

	Texture *CreateTexture2D(const TextureFormat format, const uint32_t width, const uint32_t height, const uint8_t *data2D) override {
		TextureID id = { NextID() };
		OpenGLTexture *newTexture = new OpenGLTexture(id, TextureType::T2D, format, width, height, 0, data2D);
		_texturesMap.insert(std::pair<TextureID, Texture *>(id, newTexture));
		return(newTexture);
	}


	Texture *CreateTextureCube(const TextureFormat format, const uint32_t faceWidth, const uint32_t faceHeight, const uint8_t *data2Dx6) override {
		TextureID id = { NextID() };
		OpenGLTexture *newTexture = new OpenGLTexture(id, TextureType::Cube, format, faceWidth, faceHeight, 0, data2Dx6);
		_texturesMap.insert(std::pair<TextureID, Texture *>(id, newTexture));
		return(newTexture);
	}

	void DestroyTexture(Texture &texture) override {
		OpenGLTexture *nativeTexture = reinterpret_cast<OpenGLTexture *>(&texture);
		_texturesMap.erase(texture.id);
		delete nativeTexture;
	}



};

Renderer *Renderer::Create(const RendererType type) {
	return(nullptr);
}