#include "TextureFont.h"

CTextureFont::CTextureFont(const FontAtlas &atlas):
	CTexture2D(GL_TEXTURE_2D, GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE, atlas.bitmapWidth, atlas.bitmapHeight),
	atlas(atlas) {
}

CTextureFont::~CTextureFont() {

}