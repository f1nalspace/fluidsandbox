#pragma once

#include <glad/glad.h>

#include "FontAtlas.h"

#include "Texture2D.h"

class CTextureFont : public CTexture2D {
private:
	const FontAtlas &atlas;
public:
	CTextureFont(const FontAtlas &atlas);
	~CTextureFont();
	inline const FontAtlas &GetAtlas() const { return atlas; }
};

