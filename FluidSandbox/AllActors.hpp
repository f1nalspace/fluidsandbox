#pragma once

#include "Actor.hpp"

#include <glm/glm.hpp>

struct PlaneActor: public Actor {
	PlaneActor():
		Actor(ActorType::Plane, ActorMovementType::Static) {
	}
};

struct CubeActor: public Actor {
	glm::vec3 halfExtents;

	CubeActor(const ActorMovementType movementType, const glm::vec3 &halfExtents):
		Actor(ActorType::Cube, movementType),
		halfExtents(halfExtents) {
	}
};

struct SphereActor: public Actor {
	float radius;

	SphereActor(const ActorMovementType movementType, const float radius):
		Actor(ActorType::Sphere, movementType),
		radius(radius) {

	}
};

struct CapsuleActor: public Actor {
	float radius;
	float halfHeight;

	CapsuleActor(const ActorMovementType movementType, const float radius, const float halfHeight):
		Actor(ActorType::Capsule, movementType),
		radius(radius),
		halfHeight(halfHeight) {

	}
};

struct FluidActor: public Actor {
	glm::vec3 size;
	float radius;
	float timeElapsed;
	float emitterRate;
	float emitterTime;
	float emitterElapsed;
	float emitterCoolDownElapsed;
	uint32_t emitterCoolDown;
	uint32_t emitterDuration;
	int time;
	FluidType fluidType;
	bool isEmitter;
	bool emitterCoolDownActive;

	FluidActor(const glm::vec3 &size, const float radius, const FluidType type):
		Actor(ActorType::Fluid, ActorMovementType::Dynamic),
		size(size),
		radius(radius),
		timeElapsed(0.0f),
		emitterRate(0.0f),
		emitterTime(0.0f),
		emitterElapsed(0.0f),
		emitterCoolDownElapsed(0.0f),
		emitterCoolDown(0),
		emitterDuration(0),
		time(0),
		fluidType(type),
		isEmitter(false),
		emitterCoolDownActive(false) {

	}
};