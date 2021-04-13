#pragma once

#include "Actor.hpp"

#include <glm/glm.hpp>

struct FluidActor : Actor {
};

struct CCubeActor: public Actor {
	glm::vec3 size;

	CCubeActor(const ActorType type):
		Actor(type, ActorPrimitiveKind::Cube),
		size(glm::vec3(0)) {
	}
};

struct CSphereActor: public Actor {
	float radius;

	CSphereActor(const ActorType type):
		Actor(type, ActorPrimitiveKind::Sphere),
		radius(0.5f) {

	}
};