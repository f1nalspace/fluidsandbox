#pragma once

#include "Actor.hpp"

#include <glm/glm.hpp>

struct CCubeActor: public CActor {
	glm::vec3 size;

	CCubeActor(const ActorType type):
		CActor(type, ActorPrimitiveKind::Cube),
		size(glm::vec3(0)) {
	}
};

