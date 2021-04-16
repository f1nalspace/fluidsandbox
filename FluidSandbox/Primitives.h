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
	};

	struct Primitive {
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;
		std::vector<Vertex> verts;
		std::vector<GLuint> indices;
		size_t sizeOfPositions;
		size_t sizeOfNormals;
		size_t sizeOfTexcoords;
		size_t sizeOfVertices;
		size_t vertexCount;
		size_t indexCount;
	};

	Primitive createBox(const glm::vec3 &extends);
};