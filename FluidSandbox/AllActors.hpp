/*
======================================================================================================================
	Fluid Sandbox - AllActors.hpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include "Actor.hpp"

#include <glm/glm.hpp>

struct PlaneActor: public Actor {
	PlaneActor():
		Actor(ActorType::Plane, ActorMovementType::Static) {
	}

	void Assign(const PlaneActor *source) {
		Actor::Assign(source);
	}
};

struct CubeActor: public Actor {
	glm::vec3 halfExtents;

	CubeActor(const ActorMovementType movementType, const glm::vec3 &halfExtents):
		Actor(ActorType::Cube, movementType),
		halfExtents(halfExtents) {
		assert(halfExtents.x > 0 && halfExtents.y > 0 && halfExtents.z > 0);
	}

	void Assign(const CubeActor *source) {
		Actor::Assign(source);
		halfExtents = source->halfExtents;
	}
};

struct SphereActor: public Actor {
	float radius;

	SphereActor(const ActorMovementType movementType, const float radius):
		Actor(ActorType::Sphere, movementType),
		radius(radius) {
		assert(radius > 0);
	}

	void Assign(const SphereActor *source) {
		Actor::Assign(source);
		radius = source->radius;
	}
};

struct CapsuleActor: public Actor {
	float radius;
	float halfHeight;

	CapsuleActor(const ActorMovementType movementType, const float radius, const float halfHeight):
		Actor(ActorType::Capsule, movementType),
		radius(radius),
		halfHeight(halfHeight) {
		assert(radius > 0);
		assert(halfHeight > 0);
	}

	void Assign(const CapsuleActor *source) {
		Actor::Assign(source);
		radius = source->radius;
		halfHeight = source->halfHeight;
	}
};

enum class FluidType: int {
	None = -1,
	Drop = 0,
	Plane = 1,
	Box = 2,
	Sphere = 3,
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

	void Assign(const FluidActor *source) {
		Actor::Assign(source);
		size = source->size;
		radius = source->radius;
		timeElapsed = source->timeElapsed;
		emitterRate = source->emitterRate;
		emitterTime = source->emitterTime;
		emitterElapsed = source->emitterElapsed;
		emitterCoolDownElapsed = source->emitterCoolDownElapsed;
		emitterCoolDown = source->emitterCoolDown;
		emitterDuration = source->emitterDuration;
		time = source->time;
		fluidType = source->fluidType;
		isEmitter = source->isEmitter;
		emitterCoolDownActive = source->emitterCoolDownActive;
	}
};