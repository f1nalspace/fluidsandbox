#include "SphericalPointSprites.h"

CSphericalPointSprites::CSphericalPointSprites()
{
	vboId = 0;
	totalSpriteCount = 0;
}

CSphericalPointSprites::~CSphericalPointSprites(void)
{
	if (vboId)
		glDeleteBuffersARB(1, &vboId);
}

void CSphericalPointSprites::Allocate(const unsigned int total)
{
	totalSpriteCount = total;
	glGenBuffersARB(1, &vboId);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, total * sizeof(float) * 4, nullptr, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void CSphericalPointSprites::Draw(const unsigned int count)
{
	glEnable(GL_POINT_SPRITE_ARB);
	glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboId);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glDisable(GL_POINT_SPRITE_ARB);
}

float* CSphericalPointSprites::Map()
{
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, totalSpriteCount * sizeof(float) * 4, nullptr, GL_STREAM_DRAW_ARB);
	return (float*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);
}

void CSphericalPointSprites::UnMap()
{
	glUnmapBuffer(GL_ARRAY_BUFFER_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

float CSphericalPointSprites::GetPointScale(int windowHeight, float fov) {
	return (float)(windowHeight / tan(fov * 0.5 * M_PI / 180.0));
}
