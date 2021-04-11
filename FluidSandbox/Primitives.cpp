#include "Primitives.h"
namespace Primitives
{
	CVBO* createCube(float sizeX, float sizeY, float sizeZ, bool normalPositive)
	{
		float left = -(sizeX / 2.0f);
		float right = (sizeX / 2.0f);
		float top = (sizeY / 2.0f);
		float bottom = -(sizeY / 2.0f);
		float front = (sizeZ / 2.0f);
		float back = -(sizeZ / 2.0f);

		// cube vertices for vertex buffer object
		GLfloat cube_vertices[] = {
		  left,  bottom, back,  // Left bottom back
		  right, bottom, back,  // Right bottom back
		  right, top,    back,  // Right top back
		  left,  top,    back,  // Left top back
		  left,  bottom, front, // Left bottom front
		  right, bottom, front, // Right bottom front
		  right, top,    front, // Right top front
		  left,  top,    front, // Left top front
		};
	 
		// cube indices for index buffer object
		const GLuint cube_indexCount = 36;

		GLuint cube_indices[cube_indexCount] = {
			QUAD_TO_TRIANGLE(4, 5, 1, 0), // Floor
			QUAD_TO_TRIANGLE(6, 7, 3, 2), // Ceiling
			QUAD_TO_TRIANGLE(1, 2, 3, 0), // Back
			QUAD_TO_TRIANGLE(4, 7, 6, 5), // Front
			QUAD_TO_TRIANGLE(3, 7, 4, 0), // Left side
			QUAD_TO_TRIANGLE(6, 2, 1, 5), // Right side
		};

		GLuint cube_indices_invert[cube_indexCount] = {
			QUAD_TO_TRIANGLE(0, 1, 5, 4), // Floor
			QUAD_TO_TRIANGLE(2, 3, 7, 6), // Ceiling
			QUAD_TO_TRIANGLE(0, 3, 2, 1), // Back
			QUAD_TO_TRIANGLE(5, 6, 7, 4), // Front
			QUAD_TO_TRIANGLE(0, 4, 7, 3), // Left side
			QUAD_TO_TRIANGLE(5, 1, 2, 6), // Right side
		};

		CVBO* vbo = new CVBO();
		vbo->bufferVertices(cube_vertices, sizeof(cube_vertices), GL_STATIC_DRAW);
		if (!normalPositive)
			vbo->bufferIndices(cube_indices, cube_indexCount, GL_STATIC_DRAW);
		else
			vbo->bufferIndices(cube_indices_invert, cube_indexCount, GL_STATIC_DRAW);
		return vbo;
	}
}