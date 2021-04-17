#include "TextureCubemap.h"


CTextureCubemap::CTextureCubemap(const int width, const int height)
	:CTexture2D(GL_TEXTURE_CUBE_MAP, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, width, height)
{
}

CTextureCubemap::~CTextureCubemap(void)
{
}

void CTextureCubemap::Upload(const uint8_t *pixels)
{
	GLuint newid = create();
	bind();
    glTexParameteri(getTarget(), GL_TEXTURE_MAG_FILTER, getTexMagFilter());
    glTexParameteri(getTarget(), GL_TEXTURE_MIN_FILTER, getTexMinFilter());
    glTexParameteri(getTarget(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(getTarget(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(getTarget(), GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for (int i = 0; i < 6; i++) 
	{
		const uint8_t* facePixels = &pixels[(getWidth() * getHeight() * 4) * i];
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, getInternalFormat(), getWidth(), getHeight(), 0, getFormat(), getType(), (const GLvoid*)facePixels);
	}
	unbind();
}
