#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Texture2D.h"
#include "TextureCubemap.h"
#include "TextureFont.h"
#include "FontAtlas.h"

//#define CUBEMAP_DEBUG

class CTextureManager
{
private:
	std::map<std::string, CTexture*> nameToTextureMap;
	CTexture2D* load2D(const char* filename);
	CTextureCubemap* loadCubemap(const char* filename);
public:
	CTextureManager(void);
	~CTextureManager(void);
	CTexture2D* add2D(const std::string &name, const std::string &filename);
	CTextureCubemap* addCubemap(const std::string &name, const std::string &filename);
	CTextureFont *addFont(const std::string &name, const FontAtlas &fontAtlas);
	CTexture* get(const std::string &name);
};

