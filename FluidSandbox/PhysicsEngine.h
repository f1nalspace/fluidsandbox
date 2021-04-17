/*
======================================================================================================================
	Fluid Sandbox - PhysicsEngine.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Actor.hpp"
#include "FluidProperties.h"

enum class PhysicsForceMode {
	Acceleration = 0,
	Force,
	Impulse,
	VelocityChange
};

struct PhysicsActor {
	enum class Type {
		None = 0,
		RigidBody,
		ParticleSystem,
	};

	void *userData;
	Type type;
protected:
	PhysicsActor(Type type):
		userData(nullptr),
		type(type) {
	}
};

struct PhysicsPlane {
	int unused;
};

struct PhysicsBoxShape {
	glm::vec3 halfExtents;
};

struct PhysicsSphereShape {
	float radius;
};

struct PhysicsCapsuleShape {
	float radius;
	float halfHeight;
};

struct PhysicsShape {
	enum class Type {
		None = 0,
		Plane,
		Box,
		Sphere,
		Capsule,
	};

	glm::quat localRotation;
	glm::quat worldRotation;

	glm::vec3 localPosition;
	glm::vec3 worldPosition;

	Type type;

	bool isParticleDrain;

	union {
		PhysicsPlane plane;
		PhysicsBoxShape box;
		PhysicsSphereShape sphere;
		PhysicsCapsuleShape capsule;
	};
};

struct PhysicsRigidBody: public PhysicsActor {
	constexpr static uint32_t MaxShapeCount = 8;

	enum class MotionKind {
		Static,
		Dynamic
	};

	PhysicsShape shapes[MaxShapeCount];

	glm::quat rotation;
	glm::vec3 position;
	glm::vec3 velocity;
	float density;

	uint32_t shapeCount;
	MotionKind motionKind;
protected:
	PhysicsRigidBody(const MotionKind motionKind):
		PhysicsActor(PhysicsActor::Type::RigidBody),
		position(glm::vec3(0)),
		velocity(glm::vec3(0)),
		density(1.0f),
		shapeCount(0),
		motionKind(motionKind) {
		memset(shapes, 0, sizeof(shapes));
	}
public:
	virtual void AddShape(const PhysicsShape &shape) {
		assert(shapeCount < MaxShapeCount);
		PhysicsShape *targetShape = shapes + shapeCount;
		*targetShape = shape;
		++shapeCount;
	}

	void AddPlaneShape(const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat()) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Plane;
		newShape.plane.unused = 1337;
		newShape.localPosition = localPosition;
		newShape.localRotation = localRotation;
		AddShape(newShape);
	}

	void AddBoxShape(const glm::vec3 &halfExtents, const glm::vec3 &localPosition, const glm::quat &localRotation = glm::quat()) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Box;
		newShape.box.halfExtents = halfExtents;
		newShape.localPosition = localPosition;
		newShape.localRotation = localRotation;
		AddShape(newShape);
	}

	void AddSphereShape(const float radius, const glm::vec3 &localPosition, const glm::quat &localRotation = glm::quat()) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Sphere;
		newShape.sphere.radius = radius;
		newShape.localPosition = localPosition;
		newShape.localRotation = localRotation;
		AddShape(newShape);
	}

	void AddCapsuleShape(const float radius, const float halfHeight, const glm::vec3 &localPosition, const glm::quat &localRotation = glm::quat()) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Capsule;
		newShape.capsule.radius = radius;
		newShape.capsule.halfHeight = halfHeight;
		newShape.localPosition = localPosition;
		newShape.localRotation = localRotation;
		AddShape(newShape);
	}
};

struct PhysicsParticleSystem: PhysicsActor {
	glm::vec4 *positions;
	glm::vec3 *velocities;
	uint32_t maxParticleCount;
	uint32_t activeParticleCount;
protected:
	PhysicsParticleSystem(const uint32_t maxParticleCount):
		PhysicsActor(PhysicsActor::Type::ParticleSystem),
		maxParticleCount(maxParticleCount),
		activeParticleCount(0) {
		positions = new glm::vec4[maxParticleCount];
		velocities = new glm::vec3[maxParticleCount];
	}
public:
	virtual ~PhysicsParticleSystem() {
		delete[] velocities;
		delete[] positions;
	}

	virtual void AddForce(const glm::vec3 &force, const PhysicsForceMode mode) = 0;
	virtual void SetExternalAcceleration(const glm::vec3 &accel) = 0;

	void WriteToPositionBuffer(float *dest, const size_t count) {
		assert(count < maxParticleCount);
		size_t size = sizeof(float) * 4 * count;
		memcpy_s(dest, size, &positions[0], size);
	}
};

struct PhysicsEngineConfiguration {
	uint32_t threadCount;
};

struct PhysicsParticlesStorage {
	glm::vec3 *positions;
	glm::vec3 *velocities;
	uint32_t numParticles;
};

class PhysicsEngine {
protected:
	std::vector<PhysicsActor *> actors;
	bool isInitialized;
	virtual PhysicsParticleSystem *CreateParticleSystem(const FluidSimulationProperties &desc, const uint32_t maxParticleCount) = 0;
	virtual void AddActor(PhysicsActor *actor) = 0;
	virtual void RemoveActor(PhysicsActor *actor) = 0;
	virtual PhysicsRigidBody *CreateRigidBody(const PhysicsRigidBody::MotionKind motionKind, const glm::vec3 &pos, const glm::quat &rotation, const PhysicsShape &shape) = 0;
public:
	PhysicsEngine();
	~PhysicsEngine();

	static PhysicsEngine *Create(const PhysicsEngineConfiguration &config);

	virtual void Simulate(const float deltaTime) = 0;

	virtual void Clear();

	PhysicsParticleSystem *AddParticleSystem(const FluidSimulationProperties &desc, const uint32_t maxParticleCount);
	void RemoveParticleSystem(PhysicsParticleSystem *particleSystem);
	virtual bool AddParticles(PhysicsParticleSystem *particleSystem, const PhysicsParticlesStorage &storage) = 0;

	PhysicsRigidBody *AddRigidBody(const PhysicsRigidBody::MotionKind motionKind, const glm::vec3 &pos, const glm::quat &rotation, const PhysicsShape &shape);
	void RemoveRigidBody(PhysicsRigidBody *body);

	virtual bool SupportsGPUAcceleration() = 0;
	virtual bool IsGPUAcceleration() = 0;
	virtual void SetGPUAcceleration(const bool value) = 0;
};

