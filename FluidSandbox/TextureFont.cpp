/*
======================================================================================================================
	Fluid Sandbox - TextureFont.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "TextureFont.h"

CTextureFont::CTextureFont(const FontAtlas &atlas):
	CTexture2D(GL_TEXTURE_2D, GL_RED, GL_RED, GL_UNSIGNED_BYTE, atlas.bitmapWidth, atlas.bitmapHeight),
	atlas(atlas) {
}

CTextureFont::~CTextureFont() {

}