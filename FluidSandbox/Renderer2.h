#if !defined(RENDERER2_HEADER)
#define RENDERER2_HEADER

#if !defined(__cplusplus) && ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900)))
#	define RENDERER2_IS_C99
#elif defined(__cplusplus)
#	define RENDERER2_IS_CPP
#else
#	error "This C/C++ compiler is not supported!"
#endif

#if defined(RENDERER2_IS_C99)
#	define RENDERER2_ZERO_INIT {0}
#else
#	define RENDERER2_ZERO_INIT {}
#endif
#define RENDERER2_ARRAYCOUNT(arr) (sizeof(arr) / sizeof((arr)[0]))


#if defined(RENDERER_PRIVATE)
#if defined(__cplusplus)
#define rendererapi extern "C" static
#else
#define rendererapi static
#endif
#else
#if defined(__cplusplus)
#define rendererapi extern "C" extern
#else
#define rendererapi extern
#endif
#endif

#if !defined(RENDERER_ASSERT)
#include <assert.h>
#define RENDERER_ASSERT(exp) assert(exp)
#endif
#if !defined(RENDERER_MALLOC) && !defined(RENDERER_FREE)
#include <malloc.h>
#define RENDERER_MALLOC(size) malloc(size)
#define RENDERER_FREE(ptr) free(ptr)
#endif

#include <stdint.h>

//
// RendererBuffer
//
typedef enum RendererBufferType {
	RendererBufferType_None = 0,
	RendererBufferType_Vertex,
	RendererBufferType_Index,
	RendererBufferType_Uniform,
	RendererBufferType_Storage,
} RendererBufferType;

typedef enum RendererBufferAccess {
	RendererBufferAccess_ReadWrite = 0,
	RendererBufferAccess_WriteOnly,
	RendererBufferAccess_ReadOnly,
} RendererBufferAccess;

typedef enum RendererBufferUsage {
	RendererBufferUsage_Static = 0,
	RendererBufferUsage_Dynamic,
} RendererBufferUsage;

typedef struct RendererBufferID {
	uint32_t id;
} RendererBufferID;
typedef struct RendererBuffer {
	size_t size;
	RendererBufferID id;
	RendererBufferType type;
	RendererBufferAccess access;
	RendererBufferUsage usage;
} RendererBuffer;

//
// RendererSampler
//
typedef enum RendererTextureType {
	RendererTextureType_None = 0,
	RendererTextureType_1D,
	RendererTextureType_2D,
	RendererTextureType_3D,
	RendererTextureType_Cube,
} RendererTextureType;

typedef enum RendererTextureFormat {
	None = 0,
	Alpha8,
	RGBA32,
	BGRA32,
} RendererTextureFormat;

typedef struct RendererTextureID {
	uint32_t id;
} RendererTextureID;
typedef struct RendererTexture {
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	RendererTextureID id;
	RendererTextureFormat format;
	RendererTextureType type;
} RendererTexture;

typedef struct RendererRenderTargetID {
	uint32_t id;
} RendererRenderTargetID;
typedef struct RendererRenderTarget {
	RendererTextureID texture;
	RendererRenderTargetID id;
} RendererRenderTarget;

//
// RendererShader
//
typedef enum RendererShaderType {
	RendererShaderType_None = 0,
	RendererShaderType_Vertex,
	RendererShaderType_Geometry,
	RendererShaderType_Fragment,
	RendererShaderType_Compute,
	RendererShaderType_First = RendererShaderType_Vertex,
	RendererShaderType_Last = RendererShaderType_Compute,
} RendererShaderType;

typedef struct RendererShaderID {
	uint32_t id;
} RendererShaderID;
typedef struct RendererShader {
	RendererShaderID id;
	RendererShaderType type;
} RendererShader;

static const uint32_t MaxRendererShaderTypeCount = (RendererShaderType_Last - RendererShaderType_First) + 1;
typedef struct RendererShaderProgramID {
	uint32_t id;
} RendererShaderProgramID;
typedef struct RendererShaderProgram {
	RendererShaderID shaders[MaxRendererShaderTypeCount];
	RendererShaderProgramID id;
	uint32_t shaderCount;
} RendererShaderProgram;

typedef enum RendererUniformType {
	RendererUniformType_None = 0,
	RendererUniformType_Float,
	RendererUniformType_Vec2f,
	RendererUniformType_Vec3f,
	RendererUniformType_Vec4f,
	RendererUniformType_Mat3f,
	RendererUniformType_Mat4f,
	RendererUniformType_Int,
	RendererUniformType_Vec2i,
	RendererUniformType_Vec3i,
	RendererUniformType_Vec4i,
	RendererUniformType_First = RendererUniformType_Float,
	RendererUniformType_Last = RendererUniformType_Vec4i,
} RendererUniformType;
static const uint32_t MaxRendererUniformTypeCount = (RendererUniformType_Last - RendererUniformType_First) + 1;

typedef struct RendererUniformID {
	uint32_t id;
} RendererUniformID;
typedef struct RendererUniform {
	const char *name; // Pointer to a unique uniform name
	size_t offset;
	size_t size;
	RendererUniformID id;
	RendererUniformType type;
} RendererUniform;

typedef enum RendererBufferLayoutElementType {
	RendererBufferLayoutElementType_None = 0,
	RendererBufferLayoutElementType_Float1,
	RendererBufferLayoutElementType_Float2,
	RendererBufferLayoutElementType_Float3,
	RendererBufferLayoutElementType_Float4,
	RendererBufferLayoutElementType_Int32,
	RendererBufferLayoutElementType_Byte1,
	RendererBufferLayoutElementType_Byte2,
	RendererBufferLayoutElementType_Byte3,
	RendererBufferLayoutElementType_Byte4,
} RendererBufferLayoutElementType;

typedef struct RendererBufferLayoutElementID {
	uint32_t id;
} RendererBufferLayoutElementID;
typedef struct RendererBufferLayoutElement {
	const char *name; // Pointer to a unique layout element name
	uint32_t offset;
	uint32_t count;
	uint32_t size;
	int32_t isNormalized;
	RendererBufferLayoutElementID id;
	RendererBufferLayoutElementType type;
} RendererBufferLayoutElement;

//
// RendererPipeline
//

typedef enum RendererPipelineLayoutBindingType {
	RendererPipelineLayoutBindingType_None = 0,
	RendererPipelineLayoutBindingType_UniformBuffer,
	RendererPipelineLayoutBindingType_StorageBuffer,
	RendererPipelineLayoutBindingType_Sampler
} RendererPipelineLayoutBindingType;

typedef struct RendererPipelineLayoutBindingID {
	uint32_t id;
} RendererPipelineLayoutBindingID;
typedef struct RendererPipelineLayoutBinding {
	size_t offset;
	size_t size;
	RendererPipelineLayoutBindingID id;
	RendererPipelineLayoutBindingType type;
} RendererPipelineLayoutBinding;

typedef struct RendererPipelineLayoutID {
	uint32_t id;
} RendererPipelineLayoutID;

typedef struct RendererPipelineLayout {
	RendererUniformID *uniforms;
	RendererPipelineLayoutBindingID *layoutBindings;
	RendererBufferLayoutElementID *vertexLayouts;
	uint32_t uniformCount;
	uint32_t uniformCapacity;
	uint32_t layoutBindingCount;
	uint32_t layoutBindingCapacity;
	uint32_t vertexLayoutCount;
	uint32_t vertexLayoutCapacity;
	RendererPipelineLayoutID id;
} RendererPipelineLayout;

typedef enum RendererDepthTest {
	RendererDepthTest_Off = 0,
	RendererDepthTest_On = 1
} RendererDepthTest;

typedef enum RendererCullMode {
	RendererCullMode_None = 0,
	RendererCullMode_ClockWise,
	RendererCullMode_CounterClockWise,
} RendererCullMode;

typedef enum RendererDrawMode {
	RendererDrawMode_Triangle = 0,
	RendererDrawMode_TriangleLine = 0,
	RendererDrawMode_Point = 0,
	RendererDrawMode_Line = 0,
} RendererDrawMode;

typedef struct RendererRect {
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
} RendererRect;

typedef struct RendererPipelineSettings {
	RendererDepthTest depthTest;
	RendererCullMode cullMode;
	RendererDrawMode drawMode;
} RendererPipelineSettings;

typedef struct RendererPipelineID {
	uint32_t id;
} RendererPipelineID;

typedef struct RendererPipeline {
	RendererRect viewport;
	RendererRect scissor;
	RendererPipelineSettings pipelineSettings;
	RendererPipelineLayoutID pipelineLayout;
	RendererShaderProgramID shaderProgram;
	RendererRenderTargetID renderTarget;
	RendererPipelineID id;
} RendererPipeline;

typedef struct RendererSamplerID {
	uint32_t id;
} RendererSamplerID;

typedef struct RendererSampler {
	RendererTextureID texture;
	RendererSamplerID id;
} RendererSampler;

typedef struct RendererBindingID {
	uint32_t id;
} RendererBindingID;

typedef struct RendererBinding {
	RendererBufferID *buffers;
	RendererSamplerID *samplers;
	uint32_t bufferCount;
	uint32_t bufferCapacity;
	uint32_t samplerCount;
	uint32_t samplerCapacity;
	RendererBindingID id;
} RendererBinding;

typedef enum RendererType {
	RendererType_None = 0,
	RendererType_Null,
	RendererType_OpenGL
} RendererType;

#define RENDERER_FUNC_ALLOCATE(name) void *name(const size_t size)
typedef RENDERER_FUNC_ALLOCATE(renderer_func_MemoryAllocate);
#define RENDERER_FUNC_RELEASE(name) void name(void *ptr)
typedef RENDERER_FUNC_RELEASE(renderer_func_MemoryRelease);

typedef struct RendererMemoryAllocator {
	renderer_func_MemoryAllocate *allocate;
	renderer_func_MemoryRelease *release;
} RendererMemoryAllocator;

typedef void Renderer;

typedef enum RendererCreationResult {
	RendererCreationResult_Success = 0,
	RendererCreationResult_UnsupportedRendererType,
	RendererCreationResult_InvalidAllocator,
	RendererCreationResult_OutOfMemory,
	RendererCreationResult_InvalidRendererParameter,
} RendererCreationResult;

rendererapi RendererCreationResult RendererCreate(const RendererType type, const RendererMemoryAllocator *allocator, Renderer **outRenderer);

typedef enum RendererTextureCreationResult {
	RendererTextureCreationResult_Success = 0,
	RendererTextureCreationResult_InvalidRendererParameter,
	RendererTextureCreationResult_InvalidRendererInstance,
	RendererTextureCreationResult_InvalidSizeParameter,
	RendererTextureCreationResult_InvalidFormatParameter,
	RendererTextureCreationResult_OutOfMemory,
} RendererTextureCreationResult;

rendererapi RendererTextureCreationResult RendererCreateTexture2D(Renderer *renderer, const uint32_t width, const uint32_t height, const RendererTextureFormat format, const uint8_t *data, RendererTexture **outTexture);
rendererapi void RendererDestroyTexture(Renderer *renderer, RendererTexture *texture);

rendererapi void RendererDestroy(Renderer *renderer);

#endif // RENDERER2_HEADER

#if defined(RENDERER2_IMPLEMENTATION) && !defined(RENDERER2_IMPLEMENTED)
#define RENDERER2_IMPLEMENTED

#define RZI RENDERER2_ZERO_INIT

//
// OpenGL Renderer
//
#include "glad/glad.h"

struct RendererBase;

#define RENDERER_FUNC_CREATE_TEXTURE_2D(name) RendererTextureCreationResult name(RendererBase *renderer, const uint32_t width, const uint32_t height, const RendererTextureFormat format, RendererTexture **outTexture)
typedef RENDERER_FUNC_CREATE_TEXTURE_2D(renderer_func_CreateTexture2D);
#define RENDERER_FUNC_DESTROY_TEXTURE(name) void name(RendererBase *renderer, RendererTexture *texture)
typedef RENDERER_FUNC_DESTROY_TEXTURE(renderer_func_DestroyTexture);

typedef struct RendererFunctions {
	renderer_func_CreateTexture2D *createTexture2D;
	renderer_func_DestroyTexture *destroyTexture;
} RendererFunctions;

typedef struct RendererBase {
	RendererMemoryAllocator allocator;
	RendererFunctions functions;

	RendererTexture *textures;
	uint32_t textureCapacity;
	uint32_t textureCount;

	RendererType type;
} RendererBase;

struct OpenGLRenderer;

typedef struct OpenGLRendererTexture {
	RendererTexture base;
	GLuint id;
} OpenGLRendererTexture;

static RENDERER_FUNC_CREATE_TEXTURE_2D(OpenGLRenderer__CreateTexture2D) {
	const RendererMemoryAllocator *allocator = &renderer->allocator;

	OpenGLRenderer *glRenderer = (OpenGLRenderer *)renderer;

	OpenGLRendererTexture *glTexture = (OpenGLRendererTexture *)allocator->allocate(sizeof(OpenGLRendererTexture));
	if(glTexture == NULL) return RendererTextureCreationResult_OutOfMemory;

	glTexture->base.type = RendererTextureType_2D;
	glTexture->base.width = width;
	glTexture->base.height = height;
	glTexture->base.format = format;

	*outTexture = &glTexture->base;
	return RendererTextureCreationResult_Success;
}

typedef struct OpenGLRenderer {
	RendererBase base;
} OpenGLRenderer;

static RendererCreationResult OpenGLRenderer__Create(const RendererMemoryAllocator *allocator, OpenGLRenderer **outGLRenderer) {
	OpenGLRenderer *glRenderer = (OpenGLRenderer *)allocator->allocate(sizeof(OpenGLRenderer));
	if(glRenderer == NULL) return RendererCreationResult_OutOfMemory;

	glRenderer->base.allocator = *allocator;
	glRenderer->base.type = RendererType_OpenGL;

	glRenderer->base.functions.createTexture2D = OpenGLRenderer__CreateTexture2D;

	*outGLRenderer = glRenderer;
	return RendererCreationResult_Success;
}

//
// Core
//

static RENDERER_FUNC_ALLOCATE(Renderer__DefaultAllocate) {
	void *result = RENDERER_MALLOC(size);
	return(result);
}
static RENDERER_FUNC_RELEASE(Renderer__DefaultRelease) {
	RENDERER_FREE(ptr);
}

static RendererMemoryAllocator Renderer__CreateDefaultAllocator() {
	RendererMemoryAllocator result;
	result.allocate = Renderer__DefaultAllocate;
	result.release = Renderer__DefaultRelease;
	return(result);
}

static void Renderer__Init(const RendererMemoryAllocator *allocator, Renderer *renderer) {
}

rendererapi void RendererDestroy(Renderer *renderer) {
	if(renderer == NULL) return;
	RendererBase *base = (RendererBase *)renderer;
	if(base->allocator.release != NULL) {
		base->allocator.release(renderer);
	}
}

rendererapi RendererCreationResult RendererCreate(const RendererType type, const RendererMemoryAllocator *allocator, Renderer **outRenderer) {
	if(outRenderer == NULL) {
		return RendererCreationResult_InvalidRendererParameter;
	}

	RendererMemoryAllocator activeAllocator = RENDERER2_ZERO_INIT;
	if(allocator != NULL) {
		if(allocator->allocate == NULL || allocator->release == NULL) {
			return(RendererCreationResult_InvalidAllocator);
		}
		activeAllocator = *allocator;
	} else {
		activeAllocator = Renderer__CreateDefaultAllocator();
	}
	switch(type) {
		case RendererType_OpenGL:
		{
			OpenGLRenderer *glRenderer = NULL;
			RendererCreationResult res = OpenGLRenderer__Create(&activeAllocator, &glRenderer);
			if(res != RendererCreationResult_Success) {
				if(glRenderer != NULL) {
					RendererDestroy(&glRenderer->base);
				}
				return(res);
			}
			RENDERER_ASSERT(glRenderer != NULL);
			*outRenderer = &glRenderer->base;
		} break;
		default:
			return(RendererCreationResult_UnsupportedRendererType);
	}
	RENDERER_ASSERT(*outRenderer != NULL);
	return(RendererCreationResult_Success);
}

rendererapi RendererTextureCreationResult RendererCreateTexture2D(Renderer *renderer, const uint32_t width, const uint32_t height, const RendererTextureFormat format, const uint8_t *data, RendererTexture **outTexture) {
	if(renderer == NULL) return RendererTextureCreationResult_InvalidRendererParameter;
	if(!(width > 0 && height > 0))  return RendererTextureCreationResult_InvalidSizeParameter;
	if(format == None)  return RendererTextureCreationResult_InvalidFormatParameter;
	RendererBase *base = (RendererBase *)renderer;
	if(base->functions.createTexture2D == NULL) return RendererTextureCreationResult_InvalidRendererInstance;
	RendererTextureCreationResult result = base->functions.createTexture2D(base, width, height, format, outTexture);

	return(result);
}

rendererapi void RendererDestroyTexture(Renderer *renderer, RendererTexture *texture) {
	if(renderer == NULL || texture == NULL) return;
	RendererBase *base = (RendererBase *)renderer;
	if(base->functions.destroyTexture == NULL) return;
	base->functions.destroyTexture(base, texture);
}

#endif // RENDERER2_IMPLEMENTATION