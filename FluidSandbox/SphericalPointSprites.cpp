#include "SphericalPointSprites.h"

CSphericalPointSprites::CSphericalPointSprites()
{
	iVBO = 0;
	iTotalSprites = 0;
}

CSphericalPointSprites::~CSphericalPointSprites(void)
{
	if (iVBO)
		glDeleteBuffersARB(1, &iVBO);
}

void CSphericalPointSprites::Allocate(unsigned int total)
{
	iTotalSprites = total;
	glGenBuffersARB(1, &iVBO);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, iVBO);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, total * sizeof(float) * 4, NULL, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void CSphericalPointSprites::Draw(unsigned int count)
{
	glEnable(GL_POINT_SPRITE_ARB);
	glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, iVBO);
	glVertexPointer(4, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, count);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glDisable(GL_POINT_SPRITE_ARB);
}

float* CSphericalPointSprites::Map()
{
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, iVBO);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, iTotalSprites * sizeof(float) * 4, NULL, GL_STREAM_DRAW_ARB);
	return (float*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);
}

void CSphericalPointSprites::UnMap()
{
	glUnmapBuffer(GL_ARRAY_BUFFER_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}