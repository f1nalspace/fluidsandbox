#pragma once
#include "Texture2D.h"
class CTextureCubemap: public CTexture2D
{
protected:
public:
	CTextureCubemap(const int width, const int height);
	~CTextureCubemap(void);
	virtual void Upload(char* pixels);
};
