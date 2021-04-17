/*
======================================================================================================================
	Fluid Sandbox - TextureFont.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

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

