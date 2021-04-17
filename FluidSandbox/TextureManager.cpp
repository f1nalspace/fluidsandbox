#define CUBEMAP_DEBUG

#include "TextureManager.h"

#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <final_platform_layer.h>

CTextureManager::CTextureManager(void) {
}

CTextureManager::~CTextureManager(void) {
	for(std::map<std::string, CTexture *>::const_iterator iter = nameToTextureMap.begin();
		iter != nameToTextureMap.end(); ++iter) {
		CTexture *tex = (*iter).second;
		delete tex;
		std::cout << "    Released texture '" << (*iter).first << "' successfully" << std::endl;
	}
}

struct STBBitmap {
private:
	struct LoadContext {
		fplFileHandle file;
		size_t dataSize;
		uint8_t *data;
		uint8_t *pixels;
		bool ignoreFileAndData;

		void Release() {
			if(pixels != nullptr)
				STBI_FREE(pixels);
			if(!ignoreFileAndData) {
				if(data != nullptr)
					delete[] data;
				if(file.isValid)
					fplCloseFile(&file);
			}
		}
	};

public:
	enum class FileFormat {
		None = 0,
		BMP,
		JPEG,
		PNG,
		TGA
	};

	uint8_t *pixels;
	int width;
	int height;
	int components;
	FileFormat format;

	void Release() {
		if(pixels != nullptr) {
			stbi_image_free(pixels);
			pixels = nullptr;
		}
		width = height = components = 0;
	}

	bool SaveToFile(const char *filePath, const FileFormat format, const int jpegQuality = 80) {
		if(pixels == nullptr || width == 0 || height == 0) {
			std::cerr << "Cannot write an empty/invalid image to file '" << filePath << "'!" << std::endl;
			return(false);
		}

		int writeResult = 0;
		switch(format) {
			case FileFormat::BMP:
				writeResult = stbi_write_bmp(filePath, width, height, components, pixels);
				break;
			case FileFormat::PNG:
				writeResult = stbi_write_png(filePath, width, height, components, pixels, 0);
				break;
			case FileFormat::JPEG:
				writeResult = stbi_write_jpg(filePath, width, height, components, pixels, jpegQuality);
				break;
			case FileFormat::TGA:
				writeResult = stbi_write_tga(filePath, width, height, components, pixels);
				break;
			default:
				std::cerr << "Unsupported file format '" << (int)format << "'!" << std::endl;
				return(false);
				break;
		}

		if(!writeResult) {
			std::cerr << "Failed to write image file '" << filePath << "' as '" << (int)format << "' with dimension " << width << "x" << height << "!" << std::endl;
			return(false);
		}

		return(true);
	}

	static STBBitmap Alloc(const int width, const int height, const int components) {
		STBBitmap result = {};
		result.width = width;
		result.height = height;
		result.components = components;
		size_t size = width * height * sizeof(uint8_t) * components;
		result.pixels = (uint8_t *)STBI_MALLOC(size);
		return(result);
	}

	static bool LoadFromMemory(const char *name, const uint8_t *data, const size_t size, const int requiredComponents, STBBitmap *output) {
		// NOTE(final): Context should be released automatically when functions goes out-of-scope, regardless if failed or not
		LoadContext ctx = LoadContext();
		ctx.ignoreFileAndData = true;
		ctx.data = (uint8_t *)data; // We dont release the data, so i dont care about removing const here
		ctx.dataSize = size;
		if(ctx.dataSize == 0 || ctx.data == nullptr) {
			std::cerr << "The image data '" << name << "' was invalid!" << std::endl;
			ctx.Release();
			return(false);
		}

		std::cout << "Decode image from data '" << name << "' with size of " << ctx.dataSize << " and " << requiredComponents << "components" << std::endl;
		int width = 0, height = 0, components = 0;

		assert(ctx.dataSize <= INT32_MAX);
		ctx.pixels = stbi_load_from_memory((stbi_uc const *)ctx.data, (int)ctx.dataSize, &width, &height, &components, requiredComponents);
		if(ctx.pixels == nullptr) {
			const char *err = stbi_failure_reason();
			std::cerr << "Failed to decode image from data '" << name << "': " << err << std::endl;
			ctx.Release();
			return(false);
		}

		int bitsPerPixel = components * 8;
		std::cout << "Successfully decoded image from data '" << name << "':" << std::endl;
		std::cout << "Image width: " << width << std::endl;
		std::cout << "Image height: " << height << std::endl;
		std::cout << "Image bits per pixel: " << bitsPerPixel << std::endl;

		STBBitmap bitmap = {};
		bitmap.width = width;
		bitmap.height = height;
		bitmap.components = components;
		bitmap.pixels = ctx.pixels;

		ctx.pixels = nullptr; // Do not release the loaded pixels

		ctx.Release();

		*output = bitmap;

		return(true);
	}

	static bool LoadFromFile(const char *filename, const int requiredComponents, STBBitmap *output) {
		// NOTE(final): Context should be released automatically when functions goes out-of-scope, regardless if failed or not
		LoadContext ctx = LoadContext();
		std::cout << "Load image file '" << filename << "'" << std::endl;
		if(!fplOpenBinaryFile(filename, &ctx.file)) {
			std::cerr << "Failed to load the image file '" << filename << "'!" << std::endl;
			ctx.Release();
			return(false);
		}
		ctx.dataSize = fplGetFileSizeFromHandle(&ctx.file);
		ctx.data = new uint8_t[ctx.dataSize];
		size_t read = fplReadFileBlock(&ctx.file, ctx.dataSize, ctx.data, ctx.dataSize);
		if(read != ctx.dataSize) {
			std::cerr << "Failed to load image file '" << filename << "'! Only " << read << " was read from " << ctx.dataSize << " bytes." << std::endl;
			ctx.Release();
			return(false);
		}
		std::cout << "Successfully loaded image file '" << filename << "'" << std::endl;
		fplCloseFile(&ctx.file); // File is not needed anymore

		std::cout << "  Decode image file '" << filename << "' with size of " << ctx.dataSize << std::endl;
		int width = 0, height = 0, components = 0;
		assert(ctx.dataSize <= INT32_MAX);
		ctx.pixels = stbi_load_from_memory((stbi_uc const *)ctx.data, (int)ctx.dataSize, &width, &height, &components, requiredComponents);
		if(ctx.pixels == nullptr) {
			std::cerr << "  Failed to decode image file '" << filename << "'! Maybe the format is unsupported?" << std::endl;
			ctx.Release();
			return(false);
		}
		delete[] ctx.data; // File data is not needed anymore
		ctx.data = nullptr;

		int bitsPerPixel = components * 8;
		std::cout << "  Successfully decoded image file '" << filename << "':" << std::endl;
		std::cout << "    Image width: " << width << std::endl;
		std::cout << "    Image height: " << height << std::endl;
		std::cout << "    Image bits per pixel: " << bitsPerPixel << std::endl;

		STBBitmap bitmap = {};
		bitmap.width = width;
		bitmap.height = height;
		bitmap.components = components;
		bitmap.pixels = ctx.pixels;

		ctx.pixels = nullptr; // Do not release the loaded pixels

		ctx.Release();

		*output = bitmap;

		return(true);
	}
};

CTextureCubemap *CTextureManager::loadCubemap(const char *filename) {
	const int CUBEMAPOFFSETS[6][2] = {
		{2, 1},
		{0, 1},
		{1, 0},
		{1, 2},
		{1, 1},
		{3, 1}
	};

	std::cout << "Load cubemap from file '" << filename << "'..." << std::endl;
	STBBitmap image = {};
	if(!STBBitmap::LoadFromFile(filename, 4, &image)) {
		std::cerr << "  Failed to load the cubemap from file '" << filename << "'. See previous logs for more details!" << std::endl;
		return(nullptr);
	}
	std::cout << "Successfully loaded cubemap from file '" << filename << "' successfully" << std::endl;

	CTextureCubemap *result = nullptr;

	int stride = image.width * 4;

		// Now we have to get the cubemap dimensions
	int cubemapWidth = image.width / 4;
	int cubemapHeight = image.height / 3;
	int cubemapStride = cubemapWidth * 4;

	std::cout << "    Creating cubemap faces with dimension " << cubemapWidth << "*" << cubemapHeight << std::endl;

	// Allocate 6 textures for the 6 faces and load it (32-bit)
	size_t faceSize = 4 * cubemapWidth * cubemapHeight;
	const uint8_t *pixels = image.pixels;
	assert(pixels != nullptr);
	uint8_t *texturesData = new uint8_t[faceSize * 6];
	for(int i = 0; i < 6; i++) {
		uint8_t *faceData = &texturesData[faceSize * i];
		int xOffset = CUBEMAPOFFSETS[i][0];
		int yOffset = CUBEMAPOFFSETS[i][1];

#ifdef CUBEMAP_DEBUG
		STBBitmap tempBitmap = STBBitmap::Alloc(cubemapWidth, cubemapHeight, 4);
		uint8_t *tempPixels = tempBitmap.pixels;
		assert(tempPixels != nullptr);
#endif

		int yDst = 0;
		for(int y = (yOffset * cubemapHeight); y < ((yOffset * cubemapHeight) + cubemapHeight); y++) {
			int xDst = 0;
			int yLineSrc = (image.height - 1 - y) * stride;
			int yLineDst = yDst * cubemapStride;
			for(int x = (xOffset * cubemapWidth); x < ((xOffset * cubemapWidth) + cubemapWidth); x++) {
				// RGBA
				faceData[(yLineDst + xDst * 4) + 0] = pixels[(yLineSrc + x * 4) + 0];
				faceData[(yLineDst + xDst * 4) + 1] = pixels[(yLineSrc + x * 4) + 1];
				faceData[(yLineDst + xDst * 4) + 2] = pixels[(yLineSrc + x * 4) + 2];
				faceData[(yLineDst + xDst * 4) + 3] = pixels[(yLineSrc + x * 4) + 3];
#ifdef CUBEMAP_DEBUG
				tempPixels[(yLineDst + xDst * 4) + 0] = pixels[(yLineSrc + x * 4) + 0];
				tempPixels[(yLineDst + xDst * 4) + 1] = pixels[(yLineSrc + x * 4) + 1];
				tempPixels[(yLineDst + xDst * 4) + 2] = pixels[(yLineSrc + x * 4) + 2];
				tempPixels[(yLineDst + xDst * 4) + 3] = pixels[(yLineSrc + x * 4) + 3];
#endif
				xDst++;
			}
			yDst++;
		}

#ifdef CUBEMAP_DEBUG
		size_t homePathLen = fplGetHomePath(nullptr, 0) + 1;
		std::string homePath;
		homePath.reserve(homePathLen);
		fplGetHomePath(&homePath[0], homePathLen);

		std::string folderPath;
		size_t folderPathLen = fplPathCombine(nullptr, 0, 3, homePath.c_str(), "Desktop", "cubemapSave") + 1;
		folderPath.reserve(folderPathLen);
		fplPathCombine(&folderPath[0], folderPathLen, 3, homePath.c_str(), "Desktop", "cubemapSave");

		fplDirectoriesCreate(folderPath.c_str());

		std::string savePath;
		size_t savePathLen = fplPathCombine(nullptr, 0, 2, folderPath.c_str(), "cubemap") + 1;
		savePath.reserve(savePathLen);
		fplPathCombine(&savePath[0], savePathLen, 2, folderPath.c_str(), "cubemap");

		std::string totalFilePath;
		int totalFilePathLen = fplFormatString(nullptr, 0, "%s%d.png", savePath.c_str(), i) + 1;
		totalFilePath.reserve(totalFilePathLen);
		fplFormatString(&totalFilePath[0], totalFilePathLen, "%s%d.png", savePath.c_str(), i);
		tempBitmap.SaveToFile(totalFilePath.c_str(), STBBitmap::FileFormat::PNG);
		tempBitmap.Release();
#endif
	}
	image.Release();

	result = new CTextureCubemap(cubemapWidth, cubemapHeight);
	result->Upload(texturesData);
	std::cout << "Uploaded cubemap texture -> " << result->getID() << std::endl;

	delete[] texturesData;

	return result;
}

CTexture2D *CTextureManager::load2D(const char *filename) {
	std::cout << "Load 2D texture from file '" << filename << "'..." << std::endl;
	STBBitmap image = {};
	if(!STBBitmap::LoadFromFile(filename, 4, &image)) {
		std::cerr << "Failed to load the 2D texture from file '" << filename << "'. See previous logs for more details!" << std::endl;
		return(nullptr);
	}
	std::cout << "Successfully loaded 2D texture from file '" << filename << "' successfully" << std::endl;

	uint8_t *textureData = new uint8_t[4 * image.width * image.height];
	uint8_t *pixels = image.pixels;
	for(int i = 0; i < image.width * image.height; i++) {
		textureData[i * 4 + 0] = pixels[i * 4 + 2];
		textureData[i * 4 + 1] = pixels[i * 4 + 1];
		textureData[i * 4 + 2] = pixels[i * 4 + 0];
		textureData[i * 4 + 3] = pixels[i * 4 + 3];
	}
	image.Release();

	CTexture2D *result = nullptr;
	result = new CTexture2D(GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, image.width, image.height, GL_LINEAR, GL_LINEAR);
	result->upload(textureData);
	std::cout << "Uploaded 2D texture -> " << result->getID() << std::endl;

	delete[] textureData;

	return result;
}

CTexture2D *CTextureManager::add2D(const std::string &name, const std::string &filename) {
	CTexture2D *newTexture = load2D(filename.c_str());
	nameToTextureMap.insert(std::make_pair(name, newTexture));
	return newTexture;
}

CTextureCubemap *CTextureManager::addCubemap(const std::string &name, const std::string &filename) {
	CTextureCubemap *newTexture = loadCubemap(filename.c_str());
	nameToTextureMap.insert(std::make_pair(name, newTexture));
	return newTexture;
}

CTextureFont *CTextureManager::addFont(const std::string &name, const FontAtlas &fontAtlas) {
	CTextureFont *newTexture = new CTextureFont(fontAtlas);
	nameToTextureMap.insert(std::make_pair(name, newTexture));
	std::cout << "  Loaded font texture '" << name << "' successfully" << std::endl;
	std::cout << "    Image info: " << newTexture->getWidth() << "x" << newTexture->getHeight() << std::endl;
	std::cout << "    Glyphs: " << fontAtlas.info.minChar << " - " << fontAtlas.info.maxChar << std::endl;
	std::cout << "    Font size: " << fontAtlas.info.fontSize << std::endl;

	newTexture->upload(fontAtlas.bitmap);
	std::cout << "    Uploaded font texture -> " << newTexture->getID() << std::endl;

	return(newTexture);
}

CTexture *CTextureManager::get(const std::string &name) {
	if(nameToTextureMap.count(name) > 0)
		return (*nameToTextureMap.find(name)).second;
	return nullptr;
}
