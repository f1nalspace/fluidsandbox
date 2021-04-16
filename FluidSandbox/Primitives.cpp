#include "Primitives.h"

#include <final_platform_layer.h>

#define QUAD_TO_TRIANGLE(a, b, c, d) a, b, c, c, d, a

namespace Primitives
{
	Primitive createBox(const glm::vec3 &extends)
	{
		float left = -extends.x;
		float right = extends.x;
		float top = extends.y;
		float bottom = -extends.y;
		float front = extends.z;
		float back = -extends.z;

		// cube vertices for vertex buffer object
		Vertex verts[] = {
		  Vertex(glm::vec3(left,  bottom, back), glm::vec3(0, 0, 1), glm::vec2(1.0f, 0.0f)),  // Left bottom back
		  Vertex(glm::vec3(right, bottom, back), glm::vec3(0, 0, 1), glm::vec2(0.0f, 0.0f)),  // Right bottom back
		  Vertex(glm::vec3(right, top,    back), glm::vec3(0, 0, 1), glm::vec2(0.0f, 1.0f)),  // Right top back
		  Vertex(glm::vec3(left,  top,    back), glm::vec3(0, 0, 1), glm::vec2(1.0f, 1.0f)),  // Left top back

		  Vertex(glm::vec3(left,  bottom, front), glm::vec3(0, 0, -1), glm::vec2(0.0f, 0.0f)), // Left bottom front
		  Vertex(glm::vec3(right, bottom, front), glm::vec3(0, 0, -1), glm::vec2(1.0f, 0.0f)), // Right bottom front
		  Vertex(glm::vec3(right, top,    front), glm::vec3(0, 0, -1), glm::vec2(1.0f, 1.0f)), // Right top front
		  Vertex(glm::vec3(left,  top,    front), glm::vec3(0, 0, -1), glm::vec2(0.0f, 1.0f)), // Left top front
		};
	 
		// cube indices for index buffer object
		const GLuint indexCount = 36;

		GLuint indices[indexCount] = {
			QUAD_TO_TRIANGLE(4, 5, 1, 0), // Floor
			QUAD_TO_TRIANGLE(6, 7, 3, 2), // Ceiling
			QUAD_TO_TRIANGLE(1, 2, 3, 0), // Back
			QUAD_TO_TRIANGLE(4, 7, 6, 5), // Front
			QUAD_TO_TRIANGLE(3, 7, 4, 0), // Left side
			QUAD_TO_TRIANGLE(6, 2, 1, 5), // Right side
		};

		Primitive result = Primitive();
		size_t vertexCount = fplArrayCount(verts);
		for(size_t i = 0, count = vertexCount; i < count; ++i) {
			result.positions.push_back(verts[i].pos);
			result.normals.push_back(verts[i].normal);
			result.texcoords.push_back(verts[i].texcoord);
			result.verts.push_back(verts[i]);
		}
		for(size_t i = 0; i < indexCount; ++i) {
			result.indices.push_back(indices[i]);
		}

		result.vertexCount = vertexCount;
		result.indexCount = indexCount;
		result.sizeOfPositions = sizeof(glm::vec3) * vertexCount;
		result.sizeOfNormals = sizeof(glm::vec3) * vertexCount;
		result.sizeOfTexcoords = sizeof(glm::vec2) * vertexCount;
		result.sizeOfVertices = sizeof(Vertex) * vertexCount;

		return(result);
	}

#if 0
	CVBO *createSphere(const float radius, const float height, const int sectorCount) {
		// Based on http://www.songho.ca/opengl/gl_cylinder.html
		const float PI = acosf(-1.0f);
		float sectorStep = 2 * PI / sectorCount;
		float sectorAngle;  // radian
		std::vector<glm::vec3> sideNormals;
		for(int i = 0; i <= sectorCount; ++i) {
			sectorAngle = (float)i * sectorStep;
			float x = cosf(sectorAngle);
			float y = sinf(sectorAngle);
			float z = 0;
			sideNormals.push_back(glm::vec3(x, y, z));
		}

		// put side vertices to arrays
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		for(int i = 0; i < 2; ++i) {
			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
			float t = 1.0f - i;                              // vertical tex coord; 1 to 0

			for(int j = 0, k = 0; j <= sectorCount; ++j, ++k) {
				glm::vec3 sideNormal = sideNormals[k];
				float ux = sideNormal.x;
				float uy = sideNormal.y;
				float uz = sideNormal.z;
				// position vector
				glm::vec3 vertex = glm::vec3(ux * radius, uy * radius, h);
				vertices.push_back(vertex);
				// normal vector
				glm::vec3 normal = glm::vec3(ux, uy, uz);
				normals.push_back(normal);
				// texture coordinate
				float s = (float)j / (float)sectorCount;
				glm::vec2 texcoord = glm::vec2(s, t);
				texcoords.push_back(texcoord);
			}
		}

		int baseCenterIndex = (int)vertices.size();
		int topCenterIndex = baseCenterIndex + sectorCount + 1; // include center vertex
		// put base and top vertices to arrays
		for(int i = 0; i < 2; ++i) {
			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
			float nz = -1 + i * 2;                           // z value of normal; -1 to 1

			// center point
			vertices.push_back(glm::vec3(0,0,h));
			normals.push_back(glm::vec3(0,0,nz));
			texcoords.push_back(glm::vec2(0.5f, 0.5f));

			for(int j = 0, k = 0; j < sectorCount; ++j, ++k) {
				glm::vec3 sideNormal = sideNormals[k];
				float ux = sideNormal.x;
				float uy = sideNormal.y;
				// position vector
				vertices.push_back(glm::vec3(ux * radius, uy * radius, h));
				// normal vector
				normals.push_back(glm::vec3(0, 0, nz));
				// texture coordinate
				texcoords.push_back(glm::vec2(-ux * 0.5f + 0.5f, -uy * 0.5f + 0.5f));
			}
		}

		std::vector<GLuint> indices;
		GLuint k1 = 0;                         // 1st vertex index at base
		GLuint k2 = sectorCount + 1;           // 1st vertex index at top

		// indices for the side surface
		for(int i = 0; i < sectorCount; ++i, ++k1, ++k2) {
			// 2 triangles per sector
			// k1 => k1+1 => k2
			indices.push_back(k1);
			indices.push_back(k1 + 1);
			indices.push_back(k2);

			// k2 => k1+1 => k2+1
			indices.push_back(k2);
			indices.push_back(k1 + 1);
			indices.push_back(k2 + 1);
		}

		// indices for the base surface
		//NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation
		//      please see the previous code snippet
		for(int i = 0, k = baseCenterIndex + 1; i < sectorCount; ++i, ++k) {
			if(i < sectorCount - 1) {
				indices.push_back(baseCenterIndex);
				indices.push_back(k + 1);
				indices.push_back(k);
			} else // last triangle
			{
				indices.push_back(baseCenterIndex);
				indices.push_back(baseCenterIndex + 1);
				indices.push_back(k);
			}
		}

		// indices for the top surface
		for(int i = 0, k = topCenterIndex + 1; i < sectorCount; ++i, ++k) {
			if(i < sectorCount - 1) {
				indices.push_back(topCenterIndex);
				indices.push_back(k);
				indices.push_back(k + 1);
			} else // last triangle
			{
				indices.push_back(topCenterIndex);
				indices.push_back(k);
				indices.push_back(topCenterIndex + 1);
			}
		}

		size_t verticesSize = sizeof(glm::vec3) * vertices.size();

		CVBO *vbo = new CVBO();
		vbo->bufferVertices(cube_vertices, sizeof(cube_vertices), GL_STATIC_DRAW);
		vbo->bufferIndices(cube_indices, cube_indexCount, GL_STATIC_DRAW);
		return vbo;
	}
#endif
}