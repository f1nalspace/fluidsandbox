#pragma once

#include "Actor.hpp"

#include <glm/glm.hpp>

struct FluidActor: Actor {
	FluidActor():
		Actor(ActorPrimitiveType::Fluid, ActorMovementType::Dynamic) {
	}
};

struct CubeActor: public Actor {
	glm::vec3 size;

	CubeActor(const ActorMovementType movementType):
		Actor(ActorPrimitiveType::Cube, movementType),
		size(glm::vec3(0)) {
	}
};

struct SphereActor: public Actor {
	float radius;

	SphereActor(const ActorMovementType movementType):
		Actor(ActorPrimitiveType::Sphere, movementType),
		radius(0.5f) {

	}
};

struct CapsuleActor: public Actor {
	glm::vec2 ext;

	CapsuleActor(const ActorMovementType movementType):
		Actor(ActorPrimitiveType::Capsule, movementType),
		ext(glm::vec2(0.5f, 1.0f)) {

	}
};