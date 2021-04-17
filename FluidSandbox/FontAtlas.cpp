/*
======================================================================================================================
	Fluid Sandbox - FontAtlas.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "FontAtlas.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include "OSLowLevel.h"

FontAtlas::FontAtlas():
	glyphs(nullptr),
	bitmap(nullptr),
	bitmapWidth(0),
	bitmapHeight(0){
	info = {};
}

FontAtlas::~FontAtlas() {
	if(glyphs != nullptr) {
		delete [] glyphs;
		glyphs = nullptr;
	}
	if(bitmap != nullptr) {
		delete[] bitmap;
		bitmap = nullptr;
	}
	bitmapWidth = bitmapHeight = 0;
}

FontAtlas *FontAtlas::LoadFromMemory(const uint8_t *fontData, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize, const uint32_t maxBitmapSize) {
	int fontOffset = stbtt_GetFontOffsetForIndex(fontData, fontIndex);
	if(fontOffset < 0) {
		return(nullptr);
	}

	stbtt_fontinfo fontInfo;
	if(!stbtt_InitFont(&fontInfo, fontData, fontOffset)) {
		return(nullptr);
	}

	uint32_t charCount = (maxChar - minChar) + 1;

	float pixelScale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);

	stbtt_packedchar* packedChars = new stbtt_packedchar[charCount];

	uint32_t bitmapSize = minBitmapSize;
	uint8_t *fontBitmap = nullptr;

	// @TODO(final): Support for multiple textures / codepoint-ranges, instead of finding the biggest bitmap

	// Find bitmap size where every character fits in
	int oversampleX = 2, oversampleY = 2;
	bool isPacked = false;
	do {
		fontBitmap = new uint8_t[bitmapSize * bitmapSize];

		stbtt_pack_context context;
		stbtt_PackBegin(&context, fontBitmap, bitmapSize, bitmapSize, 0, 1, nullptr);

		stbtt_PackSetOversampling(&context, oversampleX, oversampleY);

		if(stbtt_PackFontRange(&context, fontData, fontIndex, fontSize, minChar, charCount, packedChars)) {
			isPacked = true;
		} else {
			delete[] fontBitmap;
			bitmapSize *= 2;
		}

		stbtt_PackEnd(&context);
	} while(!isPacked && (bitmapSize < maxBitmapSize));

	if(fontBitmap == nullptr) {
		delete[] packedChars;
		return(nullptr);
	}

	float invAtlasW = 1.0f / (float)bitmapSize;
	float invAtlasH = 1.0f / (float)bitmapSize;

	float fontScale = 1.0f / fontSize;

	FontGlyph *glyphs = new FontGlyph[charCount];

	float spaceAdvance = 0.0f;

	for(uint32_t charIndex = 0; charIndex < charCount; ++charIndex) {
		const stbtt_packedchar *b = packedChars + charIndex;

		FontGlyph *outGlyph = glyphs + charIndex;

		outGlyph->codePoint = minChar + charIndex;

		float s0 = b->x0 * invAtlasW;
		float t0 = b->y0 * invAtlasH;
		float s1 = b->x1 * invAtlasW;
		float t1 = b->y1 * invAtlasH;

		float x0 = b->xoff * fontScale;
		float y0 = b->yoff * fontScale;
		float x1 = b->xoff2 * fontScale;
		float y1 = b->yoff2 * fontScale;

		outGlyph->offset[0] = glm::vec2(x1, y0); // Top-right
		outGlyph->offset[1] = glm::vec2(x0, y0); // Top-left
		outGlyph->offset[2] = glm::vec2(x0, y1); // Bottom-left
		outGlyph->offset[3] = glm::vec2(x1, y1); // Bottom-right

		outGlyph->uv[0] = glm::vec2(s1, t0);
		outGlyph->uv[1] = glm::vec2(s0, t0);
		outGlyph->uv[2] = glm::vec2(s0, t1);
		outGlyph->uv[3] = glm::vec2(s1, t1);

		outGlyph->advance = b->xadvance * fontScale;

		if(outGlyph->codePoint == ' ') {
			spaceAdvance = outGlyph->advance;
		}
	}

	delete[] packedChars;

	FontInfo info = {};
	info.fontSize = fontSize;
	info.minChar = minChar;
	info.maxChar = maxChar;
	info.descent = descent * pixelScale * fontScale;
	info.ascent = ascent * pixelScale * fontScale;
	info.spaceAdvance = spaceAdvance;

	FontAtlas *result = new FontAtlas();
	result->info = info;
	result->glyphs = glyphs;
	result->bitmapWidth = bitmapSize;
	result->bitmapHeight = bitmapSize;
	result->bitmap = fontBitmap;
	return(result);
}

FontAtlas *FontAtlas::LoadFromFile(const std::string &filePath, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize, const uint32_t maxBitmapSize) {
	FontAtlas *result = nullptr;
	const uint8_t *data = COSLowLevel::getBinaryFileContent(filePath);
	if(data != nullptr) {
		result = LoadFromMemory(data, fontIndex, fontSize, minChar, maxChar, minBitmapSize, maxBitmapSize);
	}
	return(result);
}