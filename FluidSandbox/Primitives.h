#pragma once

#include "VBO.h"

#include <vector>

#include <glm/glm.hpp>

namespace Primitives {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texcoord;

		Vertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &texcoord):
			pos(pos),
			normal(normal),
			texcoord(texcoord) {
		}

		float *data() {
			float *result = &pos.x;
			return(result);
		}
	};

	struct Primitive {
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		std::vector<Vertex> verts;
		std::vector<GLuint> indices;
		std::vector<GLuint> lineIndices;
		size_t sizeOfPositions;
		size_t sizeOfNormals;
		size_t sizeOfTexcoords;
		size_t sizeOfVertices;
		GLuint vertexCount;
		GLuint indexCount;
		GLuint lineIndexCount;

		void AddVertex(const glm::vec3 &pos, const glm::vec3 &normal, const glm::vec2 &texcoord) {
			positions.push_back(pos);
			normals.push_back(normal);
			texcoords.push_back(texcoord);
			verts.push_back(Vertex(pos, normal, texcoord));
		}

		void AddIndex(const GLuint index) {
			indices.push_back(index);
		}

		void AddLineIndices(const GLuint a, const GLuint b) {
			lineIndices.push_back(a);
			lineIndices.push_back(b);
		}

		void AddIndices(const GLuint a, const GLuint b, const GLuint c) {
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		}

		void ValidateAndUpdateCounts() {
			indexCount = (GLuint)indices.size();
			assert(indexCount % 3 == 0);

			lineIndexCount = (GLuint)lineIndices.size();
			assert(lineIndexCount % 2 == 0);

			vertexCount = (GLuint)verts.size();
			assert(vertexCount == positions.size());
			assert(vertexCount == texcoords.size());
			assert(vertexCount == normals.size());

			sizeOfNormals = sizeof(glm::vec3) * vertexCount;
			sizeOfPositions = sizeof(glm::vec3) * vertexCount;
			sizeOfTexcoords = sizeof(glm::vec2) * vertexCount;
			sizeOfVertices = sizeof(Vertex) * vertexCount;
		}
	};

	Primitive CreateBox(const glm::vec3 &extends, const bool invert);
	Primitive CreateSphere(const float radius, const int sectorCount, const int stackCount);
	Primitive CreateCylinder(const float baseRadius, const float topRadius, const float height, const GLuint sectorCount, const GLuint stackCount);
};