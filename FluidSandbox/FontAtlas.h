/*
======================================================================================================================
	Fluid Sandbox - FontAtlas.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <string>

#include <glm/glm.hpp>

#include <cstdint>

struct FontInfo {
	uint32_t minChar;
	uint32_t maxChar;
	float fontSize;
	float ascent;
	float descent;
	float spaceAdvance;
};

struct FontGlyph {
	// Pixel coordinates: TR, TL, BL, BR ([0, W] [0, 0] [0, H] [W, H])
	glm::vec2 uv[4]; // In range of 0.0 to 1.0
	glm::vec2 offset[4]; // In range of -1.0 to 1.0
	float advance; // In range of -1.0 to 1.0
	uint32_t codePoint; // The unicode codepoint
};

struct FontAtlas {
	FontInfo info;
	FontGlyph *glyphs;
	uint8_t *bitmap;
	uint32_t bitmapWidth;
	uint32_t bitmapHeight;
	FontAtlas();
	~FontAtlas();
	static FontAtlas *LoadFromMemory(const uint8_t *fontData, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize = 256, const uint32_t maxBitmapSize = 8192);
	static FontAtlas *LoadFromFile(const std::string &filePath, const int fontIndex, const float fontSize, const uint32_t minChar, const uint32_t maxChar, const uint32_t minBitmapSize = 256, const uint32_t maxBitmapSize = 8192);
};
