#pragma once
#include <map>
#include <string>
#include <iostream>
using namespace std;
#include "Texture2D.h"
#include "TextureCubemap.h"
#include "freeimage/FreeImage.h"

//#define CUBEMAP_DEBUG

class CTextureManager
{
private:
	map<string, CTexture*> list;
	CTexture* load2D(const char* filename);
	CTexture* loadCubemap(const char* filename);
public:
	CTextureManager(void);
	~CTextureManager(void);
	CTexture* add2D(const string &name, const string &filename);
	CTexture* addCubemap(const string &name, const string &filename);
	CTexture* get(const string &name);
};

