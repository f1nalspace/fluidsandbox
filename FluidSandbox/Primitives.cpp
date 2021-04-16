#include "Primitives.h"

#include <final_platform_layer.h>

#define _USE_MATH_DEFINES 1
#include <math.h>

#define QUAD_TO_TRIANGLE(a, b, c, d) a, b, c, c, d, a

namespace Primitives {
	Primitive CreateBox(const glm::vec3 &extends, const bool invert) {
		float w = extends.x;
		float h = extends.y;
		float d = extends.z;

#if 0
		glColor4f(1, 1, 1, 1);
		glBegin(GL_TRIANGLES);

		// front faces
		glNormal3f(0, 0, 1);
		// face v0-v1-v2
		glTexCoord2f(1, 1);  glVertex3f(1, 1, 1);
		glTexCoord2f(0, 1);  glVertex3f(-1, 1, 1);
		glTexCoord2f(0, 0);  glVertex3f(-1, -1, 1);
		// face v2-v3-v0
		glTexCoord2f(0, 0);  glVertex3f(-1, -1, 1);
		glTexCoord2f(1, 0);  glVertex3f(1, -1, 1);
		glTexCoord2f(1, 1);  glVertex3f(1, 1, 1);

		// right faces
		glNormal3f(1, 0, 0);
		// face v0-v3-v4
		glTexCoord2f(0, 1);  glVertex3f(1, 1, 1);
		glTexCoord2f(0, 0);  glVertex3f(1, -1, 1);
		glTexCoord2f(1, 0);  glVertex3f(1, -1, -1);
		// face v4-v5-v0
		glTexCoord2f(1, 0);  glVertex3f(1, -1, -1);
		glTexCoord2f(1, 1);  glVertex3f(1, 1, -1);
		glTexCoord2f(0, 1);  glVertex3f(1, 1, 1);

		// top faces
		glNormal3f(0, 1, 0);
		// face v0-v5-v6
		glTexCoord2f(1, 0);  glVertex3f(1, 1, 1);
		glTexCoord2f(1, 1);  glVertex3f(1, 1, -1);
		glTexCoord2f(0, 1);  glVertex3f(-1, 1, -1);
		// face v6-v1-v0
		glTexCoord2f(0, 1);  glVertex3f(-1, 1, -1);
		glTexCoord2f(0, 0);  glVertex3f(-1, 1, 1);
		glTexCoord2f(1, 0);  glVertex3f(1, 1, 1);

		// left faces
		glNormal3f(-1, 0, 0);
		// face  v1-v6-v7
		glTexCoord2f(1, 1);  glVertex3f(-1, 1, 1);
		glTexCoord2f(0, 1);  glVertex3f(-1, 1, -1);
		glTexCoord2f(0, 0);  glVertex3f(-1, -1, -1);
		// face v7-v2-v1
		glTexCoord2f(0, 0);  glVertex3f(-1, -1, -1);
		glTexCoord2f(1, 0);  glVertex3f(-1, -1, 1);
		glTexCoord2f(1, 1);  glVertex3f(-1, 1, 1);

		// bottom faces
		glNormal3f(0, -1, 0);
		// face v7-v4-v3
		glTexCoord2f(0, 0);  glVertex3f(-1, -1, -1);
		glTexCoord2f(1, 0);  glVertex3f(1, -1, -1);
		glTexCoord2f(1, 1);  glVertex3f(1, -1, 1);
		// face v3-v2-v7
		glTexCoord2f(1, 1);  glVertex3f(1, -1, 1);
		glTexCoord2f(0, 1);  glVertex3f(-1, -1, 1);
		glTexCoord2f(0, 0);  glVertex3f(-1, -1, -1);

		// back faces
		glNormal3f(0, 0, -1);
		// face v4-v7-v6
		glTexCoord2f(0, 0);  glVertex3f(1, -1, -1);
		glTexCoord2f(1, 0);  glVertex3f(-1, -1, -1);
		glTexCoord2f(1, 1);  glVertex3f(-1, 1, -1);
		// face v6-v5-v4
		glTexCoord2f(1, 1);  glVertex3f(-1, 1, -1);
		glTexCoord2f(0, 1);  glVertex3f(1, 1, -1);
		glTexCoord2f(0, 0);  glVertex3f(1, -1, -1);
		glEnd();
#endif

		Primitive result = Primitive();

		// Front
		size_t k = 0;
		result.AddVertex(glm::vec3(w, h, d), glm::vec3(0, 0, 1), glm::vec2(1.0f, 1.0f));
		result.AddVertex(glm::vec3(-w, h, d), glm::vec3(0, 0, 1), glm::vec2(0.0f, 1.0f));
		result.AddVertex(glm::vec3(-w, -h, d), glm::vec3(0, 0, 1), glm::vec2(0.0f, 0.0f));
		result.AddVertex(glm::vec3(w, -h, d), glm::vec3(0, 0, 1), glm::vec2(1.0f, 0.0f));
		if(invert) {
			result.AddIndices(k + 2, k + 1, k + 0);
			result.AddIndices(k + 0, k + 3, k + 2);
		} else {
			result.AddIndices(k + 0, k + 1, k + 2);
			result.AddIndices(k + 2, k + 3, k + 0);
		}

		// Right
		k = result.verts.size();
		result.AddVertex(glm::vec3(w, h, d), glm::vec3(1, 0, 0), glm::vec2(0.0f, 1.0f));
		result.AddVertex(glm::vec3(w, -h, d), glm::vec3(1, 0, 0), glm::vec2(0.0f, 0.0f));
		result.AddVertex(glm::vec3(w, -h, -d), glm::vec3(1, 0, 0), glm::vec2(1.0f, 0.0f));
		result.AddVertex(glm::vec3(w, h, -d), glm::vec3(1, 0, 0), glm::vec2(1.0f, 1.0f));
		if(invert) {
			result.AddIndices(k + 2, k + 1, k + 0);
			result.AddIndices(k + 0, k + 3, k + 2);
		} else {
			result.AddIndices(k + 0, k + 1, k + 2);
			result.AddIndices(k + 2, k + 3, k + 0);
		}

		// Top
		k = result.verts.size();
		result.AddVertex(glm::vec3(w, h, d), glm::vec3(0, 1, 0), glm::vec2(1.0f, 0.0f));
		result.AddVertex(glm::vec3(w, h, -d), glm::vec3(0, 1, 0), glm::vec2(1.0f, 1.0f));
		result.AddVertex(glm::vec3(-w, h, -d), glm::vec3(0, 1, 0), glm::vec2(0.0f, 1.0f));
		result.AddVertex(glm::vec3(-w, h, d), glm::vec3(0, 1, 0), glm::vec2(0.0f, 0.0f));
		if(invert) {
			result.AddIndices(k + 2, k + 1, k + 0);
			result.AddIndices(k + 0, k + 3, k + 2);
		} else {
			result.AddIndices(k + 0, k + 1, k + 2);
			result.AddIndices(k + 2, k + 3, k + 0);
		}

		// Left
		k = result.verts.size();
		result.AddVertex(glm::vec3(-w, h, d), glm::vec3(-1, 0, 0), glm::vec2(1.0f, 1.0f));
		result.AddVertex(glm::vec3(-w, h, -d), glm::vec3(-1, 0, 0), glm::vec2(0.0f, 1.0f));
		result.AddVertex(glm::vec3(-w, -h, -d), glm::vec3(-1, 0, 0), glm::vec2(0.0f, 0.0f));
		result.AddVertex(glm::vec3(-w, -h, d), glm::vec3(-1, 0, 0), glm::vec2(1.0f, 0.0f));
		if(invert) {
			result.AddIndices(k + 2, k + 1, k + 0);
			result.AddIndices(k + 0, k + 3, k + 2);
		} else {
			result.AddIndices(k + 0, k + 1, k + 2);
			result.AddIndices(k + 2, k + 3, k + 0);
		}

		// Bottom
		k = result.verts.size();
		result.AddVertex(glm::vec3(-w, -h, -d), glm::vec3(0, -1, 0), glm::vec2(0.0f, 0.0f));
		result.AddVertex(glm::vec3(w, -h, -d), glm::vec3(0, -1, 0), glm::vec2(1.0f, 0.0f));
		result.AddVertex(glm::vec3(w, -h, d), glm::vec3(0, -1, 0), glm::vec2(1.0f, 1.0f));
		result.AddVertex(glm::vec3(-w, -h, d), glm::vec3(0, -1, 0), glm::vec2(0.0f, 1.0f));
		if(invert) {
			result.AddIndices(k + 2, k + 1, k + 0);
			result.AddIndices(k + 0, k + 3, k + 2);
		} else {
			result.AddIndices(k + 0, k + 1, k + 2);
			result.AddIndices(k + 2, k + 3, k + 0);
		}

		// Back
		k = result.verts.size();
		result.AddVertex(glm::vec3(w, -h, -d), glm::vec3(0, 0, -1), glm::vec2(0.0f, 0.0f));
		result.AddVertex(glm::vec3(-w, -h, -d), glm::vec3(0, 0, -1), glm::vec2(1.0f, 0.0f));
		result.AddVertex(glm::vec3(-w, h, -d), glm::vec3(0, 0, -1), glm::vec2(1.0f, 1.0f));
		result.AddVertex(glm::vec3(w, h, -d), glm::vec3(0, 0, -1), glm::vec2(0.0f, 1.0f));
		if(invert) {
			result.AddIndices(k + 2, k + 1, k + 0);
			result.AddIndices(k + 0, k + 3, k + 2);
		} else {
			result.AddIndices(k + 0, k + 1, k + 2);
			result.AddIndices(k + 2, k + 3, k + 0);
		}

		result.ValidateAndUpdateCounts();

		return(result);
	}

	Primitive CreateSphere(const float radius, const int sectorCount, const int stackCount) {
		assert(radius > 0);
		assert(sectorCount > 0);
		assert(stackCount > 0);

		Primitive result = Primitive();

		const float PI = acosf(-1.0f);
		float x, y, z, xy;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
		float s, t;                                     // vertex texCoord

		float sectorStep = 2 * PI / sectorCount;
		float stackStep = PI / stackCount;
		float sectorAngle, stackAngle;

		for(int i = 0; i <= stackCount; ++i) {
			stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xy = radius * cosf(stackAngle);             // r * cos(u)
			z = radius * sinf(stackAngle);              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for(int j = 0; j <= sectorCount; ++j) {
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				// vertex position (x, y, z)
				x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
				y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
				glm::vec3 pos = glm::vec3(x, y, z);

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				glm::vec3 normal = glm::vec3(nx, ny, nz);

				// vertex tex coord (s, t) range between [0, 1]
				s = (float)j / sectorCount;
				t = (float)i / stackCount;
				glm::vec2 texcoord = glm::vec2(s, t);

				result.AddVertex(pos, normal, texcoord);
			}
		}

		// generate CCW index list of sphere triangles
		// k1--k1+1
		// |  / |
		// | /  |
		// k2--k2+1
		int k1, k2;
		for(int i = 0; i < stackCount; ++i) {
			k1 = i * (sectorCount + 1);     // beginning of current stack
			k2 = k1 + sectorCount + 1;      // beginning of next stack

			for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if(i != 0) {
					result.AddIndices(k1, k2, k1 + 1);
				}

				// k1+1 => k2 => k2+1
				if(i != (stackCount - 1)) {
					result.AddIndices(k1 + 1, k2, k2 + 1);
				}

#if 0
				// store indices for lines
				// vertical lines for all stacks, k1 => k2
				lineIndices.push_back(k1);
				lineIndices.push_back(k2);
				if(i != 0)  // horizontal lines except 1st stack, k1 => k+1
				{
					lineIndices.push_back(k1);
					lineIndices.push_back(k1 + 1);
				}
#endif
			}
		}

		result.ValidateAndUpdateCounts();

		return(result);
	}

	Primitive CreateCylinder(const float baseRadius, const float topRadius, const float height, const GLuint sectorCount, const GLuint stackCount) {
		// Based on http://www.songho.ca/opengl/gl_cylinder.html

		assert(baseRadius > 0);
		assert(topRadius > 0);
		assert(height > 0);
		assert(sectorCount > 0);
		assert(stackCount > 0);

		Primitive result = Primitive();

		// Compute side normals for the XY-Plane
		const float PI = acosf(-1.0f);
		float sectorStep = 2 * PI / sectorCount;
		float sectorAngle;  // radian
		std::vector<glm::vec3> sideNormals;
		for(GLuint i = 0; i <= sectorCount; ++i) {
			sectorAngle = (float)i * sectorStep;
			float x = cosf(sectorAngle);
			float y = sinf(sectorAngle);
			float z = 0;
			sideNormals.push_back(glm::vec3(x, y, z));
		}

		float x, y, z; // vertex position
		float radius;  // radius for each stack

		// Put vertices of side cylinder to array by scaling unit circle
		for(GLuint i = 0; i <= stackCount; ++i) {
			z = -(height * 0.5f) + (float)i / stackCount * height;      // vertex position z
			radius = baseRadius + (float)i / stackCount * (topRadius - baseRadius);     // lerp
			float t = 1.0f - (float)i / stackCount;   // top-to-bottom
			for(GLuint j = 0, k = 0; j <= sectorCount; ++j, ++k) {
				glm::vec3 sideNormal = sideNormals[k];
				x = sideNormal.x;
				y = sideNormal.y;
				glm::vec3 pos = glm::vec3(x * radius, y * radius, z);
				glm::vec3 normal = glm::vec3(sideNormal);
				glm::vec2 texcoord = glm::vec2((float)j / sectorCount, t);
				result.AddVertex(pos, normal, texcoord);
			}
		}

		// Remember where the base.top vertices start
		GLuint baseVertexIndex = (GLuint)result.positions.size();

		// Put vertices of base of cylinder
		z = -height * 0.5f;
		result.AddVertex(glm::vec3(0, 0, z), glm::vec3(0, 0, -1), glm::vec2(0.5f, 0.5f));
		for(GLuint i = 0, j = 0; i < sectorCount; ++i, ++j) {
			glm::vec3 sideNormal = sideNormals[j];
			x = sideNormal.x;
			y = sideNormal.y;
			glm::vec3 pos = glm::vec3(x * baseRadius, y * baseRadius, z);
			glm::vec3 normal = glm::vec3(0, 0, -1);
			glm::vec2 texcoord = glm::vec2(-x * 0.5f + 0.5f, -y * 0.5f + 0.5f); // flip horizontal
			result.AddVertex(pos, normal, texcoord);
		}

		// remember where the base vertices start
		GLuint topVertexIndex = (GLuint)result.positions.size();

		// put vertices of top of cylinder
		z = height * 0.5f;
		result.AddVertex(glm::vec3(0, 0, z), glm::vec3(0, 0, 1), glm::vec2(0.5f, 0.5f));
		for(GLuint i = 0, j = 0; i < sectorCount; ++i, ++j) {
			glm::vec3 sideNormal = sideNormals[j];
			x = sideNormal.x;
			y = sideNormal.y;
			glm::vec3 pos = glm::vec3(x * topRadius, y * topRadius, z);
			glm::vec3 normal = glm::vec3(0, 0, 1);
			glm::vec2 texcoord = glm::vec2(x * 0.5f + 0.5f, -y * 0.5f + 0.5f);
			result.AddVertex(pos, normal, texcoord);
		}

		// put indices for sides
		GLuint k1, k2;
		for(GLuint i = 0; i < stackCount; ++i) {
			k1 = i * (sectorCount + 1);     // beginning of current stack
			k2 = k1 + sectorCount + 1;      // beginning of next stack

			for(GLuint j = 0; j < sectorCount; ++j, ++k1, ++k2) {
				// 2 triangles per sector
				result.AddIndices(k1, k1 + 1, k2);
				result.AddIndices(k2, k1 + 1, k2 + 1);

#if 0
				// vertical lines for all stacks
				lineIndices.push_back(k1);
				lineIndices.push_back(k2);
				// horizontal lines
				lineIndices.push_back(k2);
				lineIndices.push_back(k2 + 1);
				if(i == 0) {
					lineIndices.push_back(k1);
					lineIndices.push_back(k1 + 1);
				}
#endif
	}
}

// remember where the base indices start
		GLuint baseIndex = (GLuint)result.indices.size();

		// put indices for base
		for(GLuint i = 0, k = baseVertexIndex + 1; i < sectorCount; ++i, ++k) {
			if(i < (sectorCount - 1))
				result.AddIndices(baseVertexIndex, k + 1, k);
			else
				result.AddIndices(baseVertexIndex, baseVertexIndex + 1, k); // last triangle
		}

		// remember where the base indices start
		GLuint topIndex = (GLuint)result.indices.size();

		for(GLuint i = 0, k = topVertexIndex + 1; i < sectorCount; ++i, ++k) {
			if(i < (sectorCount - 1))
				result.AddIndices(topVertexIndex, k, k + 1);
			else
				result.AddIndices(topVertexIndex, k, topVertexIndex + 1);
		}

#if 0
		// put side vertices to arrays
		for(int i = 0; i < 2; ++i) {
			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
			float t = 1.0f - i;                              // vertical tex coord; 1 to 0
			for(int j = 0, k = 0; j <= sectorCount; ++j, ++k) {
				glm::vec3 sideNormal = sideNormals[k];
				float ux = sideNormal.x;
				float uy = sideNormal.y;
				float uz = sideNormal.z;
				glm::vec3 pos = glm::vec3(ux * radius, uy * radius, h);
				glm::vec3 normal = glm::vec3(ux, uy, uz);
				glm::vec2 texcoord = glm::vec2((float)j / (float)sectorCount, t);
				result.positions.push_back(pos);
				result.normals.push_back(normal);
				result.texcoords.push_back(texcoord);
				result.verts.push_back(Vertex(pos, normal, texcoord));
			}
		}

		int baseCenterIndex = (int)result.positions.size();
		int topCenterIndex = baseCenterIndex + sectorCount + 1; // include center vertex
		// put base and top vertices to arrays
		for(int i = 0; i < 2; ++i) {
			float h = -height / 2.0f + i * height;           // z value; -h/2 to h/2
			float nz = -1 + i * 2;                           // z value of normal; -1 to 1

			// center point
			glm::vec3 centerPos = glm::vec3(0, 0, h);
			glm::vec3 centerNormal = glm::vec3(0, 0, nz);
			glm::vec2 centerTexcoord = glm::vec2(0.5f, 0.5f);
			result.positions.push_back(centerPos);
			result.normals.push_back(centerNormal);
			result.texcoords.push_back(centerTexcoord);
			result.verts.push_back(Vertex(centerPos, centerNormal, centerTexcoord));

			for(int j = 0, k = 0; j < sectorCount; ++j, ++k) {
				glm::vec3 sideNormal = sideNormals[k];
				float ux = sideNormal.x;
				float uy = sideNormal.y;
				glm::vec3 pos = glm::vec3(ux * radius, uy * radius, h);
				glm::vec3 normal = glm::vec3(0, 0, nz);
				glm::vec2 texcoord = glm::vec2(-ux * 0.5f + 0.5f, -uy * 0.5f + 0.5f);
				result.positions.push_back(pos);
				result.normals.push_back(normal);
				result.texcoords.push_back(texcoord);
				result.verts.push_back(Vertex(pos, normal, texcoord));
			}
		}

		GLuint k1 = 0;                         // 1st vertex index at base
		GLuint k2 = sectorCount + 1;           // 1st vertex index at top

		// indices for the side surface
		for(GLuint i = 0; i < sectorCount; ++i, ++k1, ++k2) {
			// 2 triangles per sector
			// k1 => k1+1 => k2
			result.indices.push_back(k1);
			result.indices.push_back(k1 + 1);
			result.indices.push_back(k2);

			// k2 => k1+1 => k2+1
			result.indices.push_back(k2);
			result.indices.push_back(k1 + 1);
			result.indices.push_back(k2 + 1);
		}

		// indices for the base surface
		//NOTE: baseCenterIndex and topCenterIndices are pre-computed during vertex generation
		//      please see the previous code snippet
		for(GLuint i = 0, k = baseCenterIndex + 1; i < sectorCount; ++i, ++k) {
			if(i < sectorCount - 1) {
				result.indices.push_back(baseCenterIndex);
				result.indices.push_back(k + 1);
				result.indices.push_back(k);
			} else // last triangle
			{
				result.indices.push_back(baseCenterIndex);
				result.indices.push_back(baseCenterIndex + 1);
				result.indices.push_back(k);
			}
		}

		// indices for the top surface
		for(GLuint i = 0, k = topCenterIndex + 1; i < sectorCount; ++i, ++k) {
			if(i < sectorCount - 1) {
				result.indices.push_back(topCenterIndex);
				result.indices.push_back(k);
				result.indices.push_back(k + 1);
			} else // last triangle
			{
				result.indices.push_back(topCenterIndex);
				result.indices.push_back(k);
				result.indices.push_back(topCenterIndex + 1);
			}
		}
#endif

		result.ValidateAndUpdateCounts();

		return(result);
	}
}