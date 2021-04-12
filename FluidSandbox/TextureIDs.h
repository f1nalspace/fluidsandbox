#pragma once

#include <stdint.h>

typedef uint32_t TextureID;

constexpr TextureID TEXTURE_ID_SSF_DEPTH = 0;
constexpr TextureID TEXTURE_ID_SSF_COLOR = 1;
constexpr TextureID TEXTURE_ID_SSF_THICKNESS = 2;
constexpr TextureID TEXTURE_ID_SSF_DEPTH_SMOOTH_A = 3;
constexpr TextureID TEXTURE_ID_SSF_DEPTH_SMOOTH_B = 4;
constexpr TextureID TEXTURE_ID_SSF_WATER = 5;

constexpr TextureID TEXTURE_ID_SCENE_SCENE = 0;
constexpr TextureID TEXTURE_ID_SCENE_DEPTH = 1;
