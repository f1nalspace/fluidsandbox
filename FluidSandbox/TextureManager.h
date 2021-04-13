#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Texture2D.h"
#include "TextureCubemap.h"

//#define CUBEMAP_DEBUG

class CTextureManager
{
private:
	std::map<std::string, CTexture*> nameToTextureMap;
	CTexture* load2D(const char* filename);
	CTexture* loadCubemap(const char* filename);
public:
	CTextureManager(void);
	~CTextureManager(void);
	CTexture* add2D(const std::string &name, const std::string &filename);
	CTexture* addCubemap(const std::string &name, const std::string &filename);
	CTexture* get(const std::string &name);
};

