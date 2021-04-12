#include "TextureManager.h"

#include <freeimage/FreeImage.h>

CTextureManager::CTextureManager(void) {
}

CTextureManager::~CTextureManager(void) {
	for(std::map<std::string, CTexture *>::const_iterator iter = list.begin();
		iter != list.end(); ++iter) {
		CTexture *tex = (*iter).second;
		delete tex;
		std::cout << "    Released texture '" << (*iter).first << "' successfully" << std::endl;
	}
}

CTexture *CTextureManager::loadCubemap(const char *filename) {
	const int CUBEMAPOFFSETS[6][2] = {
		{2, 1},
		{0, 1},
		{1, 0},
		{1, 2},
		{1, 1},
		{3, 1}
	};

	CTextureCubemap *result = NULL;

	FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType(filename);
	FIBITMAP *bitmap = FreeImage_Load(imageFormat, filename);
	if(bitmap) {
		std::cout << "  Loaded cubemap '" << filename << "' successfully" << std::endl;

		FIBITMAP *temp = bitmap;
		bitmap = FreeImage_ConvertTo32Bits(bitmap);
		FreeImage_Unload(temp);

		int w = FreeImage_GetWidth(bitmap);
		int h = FreeImage_GetHeight(bitmap);
		unsigned int bpp = FreeImage_GetBPP(bitmap);
		std::cout << "    Image info: " << w << "x" << h << "x" << bpp << std::endl;

		if(bpp < 32) {
			std::cout << "    Convert to 32 bit image" << std::endl;
			temp = bitmap;
			bitmap = FreeImage_ConvertTo32Bits(bitmap);
			FreeImage_Unload(temp);
		}

		int stride = w * 4;

		// Now we have to get the cubemap dimensions
		int cubemapWidth = w / 4;
		int cubemapHeight = h / 3;
		int cubemapStride = cubemapWidth * 4;

		std::cout << "    Creating cubemap faces with dimension " << cubemapWidth << "*" << cubemapHeight << std::endl;

		// Allocate 6 textures for the 6 faces and load it
		char *pixels = (char *)FreeImage_GetBits(bitmap);
		char *texturesData = new char[(4 * cubemapWidth * cubemapHeight) * 6];
		for(int i = 0; i < 6; i++) {
			char *curTextureData = &texturesData[(4 * cubemapWidth * cubemapHeight) * i];
			int xOffset = CUBEMAPOFFSETS[i][0];
			int yOffset = CUBEMAPOFFSETS[i][1];
			int yidx = 0;

#ifdef CUBEMAP_DEBUG
			FIBITMAP *newbmp = FreeImage_Allocate(cubemapWidth, cubemapHeight, 32);
			char *pixelsNew = (char *)FreeImage_GetBits(newbmp);
#endif

			for(int y = (yOffset * cubemapHeight); y < ((yOffset * cubemapHeight) + cubemapHeight); y++) {
				int xidx = 0;
				for(int x = (xOffset * cubemapWidth); x < ((xOffset * cubemapWidth) + cubemapWidth); x++) {
					curTextureData[((yidx * cubemapStride) + xidx * 4) + 0] = pixels[((y * stride) + x * 4) + 2];
					curTextureData[((yidx * cubemapStride) + xidx * 4) + 1] = pixels[((y * stride) + x * 4) + 1];
					curTextureData[((yidx * cubemapStride) + xidx * 4) + 2] = pixels[((y * stride) + x * 4) + 0];
					curTextureData[((yidx * cubemapStride) + xidx * 4) + 3] = pixels[((y * stride) + x * 4) + 3];

#ifdef CUBEMAP_DEBUG
					pixelsNew[((yidx * cubemapStride) + xidx * 4) + 2] = pixels[((y * stride) + x * 4) + 2];
					pixelsNew[((yidx * cubemapStride) + xidx * 4) + 1] = pixels[((y * stride) + x * 4) + 1];
					pixelsNew[((yidx * cubemapStride) + xidx * 4) + 0] = pixels[((y * stride) + x * 4) + 0];
					pixelsNew[((yidx * cubemapStride) + xidx * 4) + 3] = pixels[((y * stride) + x * 4) + 3];
#endif

					xidx++;
				}
				yidx++;
			}

#ifdef CUBEMAP_DEBUG			
			const char *path = "C:\\Users\\final\\Desktop\\cubemapSave\\cubemap";
			char buffer[255];
			sprintf_s(buffer, "%s%d.png", path, i);
			FreeImage_Save(FIF_PNG, newbmp, buffer, 0);
			FreeImage_Unload(newbmp);
#endif
		}

		result = new CTextureCubemap(cubemapWidth, cubemapHeight);
		result->Upload(texturesData);
		std::cout << "    Uploaded cubemap texture -> " << result->getID() << std::endl;

		delete texturesData;
		FreeImage_Unload(bitmap);

	} else {
		std::cerr << "  Cubemap '" << filename << "' could not be loaded!" << std::endl;
	}

	return result;
}

CTexture *CTextureManager::load2D(const char *filename) {
	const GLuint DEFAULT_TEXTURE_FILTERS[2] = { GL_LINEAR, GL_LINEAR };

	CTexture2D *result = NULL;

	FREE_IMAGE_FORMAT imageFormat = FreeImage_GetFileType(filename);
	FIBITMAP *bitmap = FreeImage_Load(imageFormat, filename);
	if(bitmap) {
		std::cout << "  Loaded image '" << filename << "' successfully" << std::endl;

		FIBITMAP *temp = bitmap;
		bitmap = FreeImage_ConvertTo32Bits(bitmap);
		FreeImage_Unload(temp);

		int w = FreeImage_GetWidth(bitmap);
		int h = FreeImage_GetHeight(bitmap);
		unsigned int bpp = FreeImage_GetBPP(bitmap);
		std::cout << "    Image info: " << w << "x" << h << "x" << bpp << std::endl;

		if(bpp < 32) {
			std::cout << "    Convert to 32 bit image" << std::endl;
			temp = bitmap;
			bitmap = FreeImage_ConvertTo32Bits(bitmap);
			FreeImage_Unload(temp);
		}

		char *textureData = new char[4 * w * h];
		char *pixels = (char *)FreeImage_GetBits(bitmap);
		for(int i = 0; i < w * h; i++) {
			textureData[i * 4 + 0] = pixels[i * 4 + 2];
			textureData[i * 4 + 1] = pixels[i * 4 + 1];
			textureData[i * 4 + 2] = pixels[i * 4 + 0];
			textureData[i * 4 + 3] = pixels[i * 4 + 3];
		}
		FreeImage_Unload(bitmap);

		result = new CTexture2D(GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, (GLuint *)&DEFAULT_TEXTURE_FILTERS[0]);
		result->upload(textureData);
		std::cout << "    Uploaded cubemap texture -> " << result->getID() << std::endl;

		delete textureData;

	} else {
		std::cerr << "  Image '" << filename << "' could not be loaded!" << std::endl;
	}

	return result;
}

CTexture *CTextureManager::add2D(const std::string &name, const std::string &filename) {
	CTexture *newTexture = load2D(filename.c_str());
	list.insert(std::make_pair(name, newTexture));
	return newTexture;
}

CTexture *CTextureManager::addCubemap(const std::string &name, const std::string &filename) {
	CTexture *newTexture = loadCubemap(filename.c_str());
	list.insert(std::make_pair(name, newTexture));
	return newTexture;
}

CTexture *CTextureManager::get(const std::string &name) {
	if(list.count(name) > 0)
		return (*list.find(name)).second;
	return NULL;
}
