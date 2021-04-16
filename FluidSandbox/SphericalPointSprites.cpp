#include "SphericalPointSprites.h"

CSphericalPointSprites::CSphericalPointSprites()
{
	vboId = 0;
	totalSpriteCount = 0;
}

CSphericalPointSprites::~CSphericalPointSprites(void)
{
	if (vboId)
		glDeleteBuffers(1, &vboId);
}

void CSphericalPointSprites::Allocate(const unsigned int total)
{
	totalSpriteCount = total;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, total * sizeof(float) * 4, nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CSphericalPointSprites::Draw(const unsigned int count)
{
	glEnable(GL_POINT_SPRITE);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_POINT_SPRITE);
}

float* CSphericalPointSprites::Map()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, totalSpriteCount * sizeof(float) * 4, nullptr, GL_STREAM_DRAW);
	return (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
}

void CSphericalPointSprites::UnMap()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

float CSphericalPointSprites::GetPointScale(int windowHeight, float fov) {
	return (float)(windowHeight / tan(fov * 0.5 * M_PI / 180.0));
}
