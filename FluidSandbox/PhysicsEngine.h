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

enum class PhysicsForceMode: int {
	Acceleration = 0,
	Force,
	Impulse,
	VelocityChange
};

struct PhysicsTransform {
	glm::vec3 pos;
	glm::quat rotation;

	PhysicsTransform(const glm::vec3 &pos, const glm::quat &rotation):
		pos(pos),
		rotation(rotation) {
	}

	PhysicsTransform(): PhysicsTransform(glm::vec3(0), glm::quat(glm::vec3(0))) {}
};

struct PhysicsBoundingBox {
	glm::vec3 min;
	glm::vec3 max;

	PhysicsBoundingBox(const glm::vec3 &min, const glm::vec3 &max):
		min(min),
		max(max) {
	}

	PhysicsBoundingBox(): PhysicsBoundingBox(glm::vec3(0), glm::vec3(0)) {}

	inline glm::vec3 GetCenter() const {
		glm::vec3 result = min + (max - min) * 0.5f;
		return(result);
	}

	inline glm::vec3 GetSize() const {
		glm::vec3 result = max - min;
		return(result);
	}
};

struct PhysicsActor {
public:
	enum class Type {
		None = 0,
		RigidBody,
		ParticleSystem,
	};
	PhysicsTransform transform;
	PhysicsBoundingBox bounds;
	void *userData;
	Type type;
	bool isReady;
protected:
	PhysicsActor(Type type):
		transform(PhysicsTransform()),
		bounds(PhysicsBoundingBox()),
		userData(nullptr),
		type(type),
		isReady(false) {
	}
public:
	virtual ~PhysicsActor() {}
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

	PhysicsTransform local;

	Type type;

	bool isParticleDrain;

	union {
		PhysicsPlane plane;
		PhysicsBoxShape box;
		PhysicsSphereShape sphere;
		PhysicsCapsuleShape capsule;
	};

	static PhysicsShape MakePlane(const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Plane;
		newShape.plane.unused = 1337;
		newShape.local.pos = localPosition;
		newShape.local.rotation = localRotation;
		return(newShape);
	}

	static PhysicsShape MakeBox(const glm::vec3 &halfExtents, const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Box;
		newShape.box.halfExtents = halfExtents;
		newShape.local.pos = localPosition;
		newShape.local.rotation = localRotation;
		return(newShape);
	}

	static PhysicsShape MakeSphere(const float radius, const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Sphere;
		newShape.sphere.radius = radius;
		newShape.local.pos = localPosition;
		newShape.local.rotation = localRotation;
		return(newShape);
	}

	static PhysicsShape MakeCapsule(const float radius, const float halfHeight, const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = {};
		newShape.type = PhysicsShape::Type::Capsule;
		newShape.capsule.radius = radius;
		newShape.capsule.halfHeight = halfHeight;
		newShape.local.pos = localPosition;
		newShape.local.rotation = localRotation;
		return(newShape);
	}
};

struct PhysicsRigidBody: public PhysicsActor {
	constexpr static uint32_t MaxShapeCount = 8;

	enum class MotionKind {
		Static,
		Dynamic
	};

	PhysicsShape shapes[MaxShapeCount];

	glm::vec3 velocity;
	float density;

	uint32_t shapeCount;
	MotionKind motionKind;
protected:
	PhysicsRigidBody(const MotionKind motionKind):
		PhysicsActor(PhysicsActor::Type::RigidBody),
		velocity(glm::vec3(0)),
		density(1.0f),
		shapeCount(0),
		motionKind(motionKind) {
		memset(shapes, 0, sizeof(shapes));
	}
public:
	virtual ~PhysicsRigidBody() {}

	virtual void AddShape(const PhysicsShape &shape) {
		assert(shapeCount < MaxShapeCount);
		PhysicsShape *targetShape = shapes + shapeCount;
		*targetShape = shape;
		++shapeCount;
	}

	void AddPlaneShape(const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = PhysicsShape::MakePlane(localPosition, localRotation);
		AddShape(newShape);
	}

	void AddBoxShape(const glm::vec3 &halfExtents, const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = PhysicsShape::MakeBox(halfExtents, localPosition, localRotation);
		AddShape(newShape);
	}

	void AddSphereShape(const float radius, const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = PhysicsShape::MakeSphere(radius, localPosition, localRotation);
		AddShape(newShape);
	}

	void AddCapsuleShape(const float radius, const float halfHeight, const glm::vec3 &localPosition = glm::vec3(0), const glm::quat &localRotation = glm::quat(glm::vec3(0))) {
		PhysicsShape newShape = PhysicsShape::MakeCapsule(radius, halfHeight, localPosition, localRotation);
		AddShape(newShape);
	}
};

struct PhysicsParticleSystem: PhysicsActor {
	glm::vec3 *positions;
	glm::vec3 *velocities;
	float *densities;
	uint32_t maxParticleCount;
	uint32_t activeParticleCount;
protected:
	PhysicsParticleSystem(const uint32_t maxParticleCount):
		PhysicsActor(PhysicsActor::Type::ParticleSystem),
		maxParticleCount(maxParticleCount),
		activeParticleCount(0) {
		positions = new glm::vec3[maxParticleCount];
		velocities = new glm::vec3[maxParticleCount];
		densities = new float[maxParticleCount];
	}
public:
	virtual ~PhysicsParticleSystem() {
		delete[] densities;
		delete[] velocities;
		delete[] positions;
	}

	virtual void AddForce(const glm::vec3 &force, const PhysicsForceMode mode) = 0;
	virtual void SetExternalAcceleration(const glm::vec3 &accel) = 0;

	void WriteToPositionBuffer(float *dest, const size_t maxCount, const bool noDensity, const float minDensity) {
		assert(maxCount <= maxParticleCount);
		for(size_t i = 0; i < activeParticleCount; ++i) {
			float x = positions[i].x;
			float y = positions[i].y;
			float z = positions[i].z;
			float w = densities[i];
			if(!noDensity) {
				if(w < minDensity) w = minDensity;
				if(w > 1.0f) w = 1.0f;
			} else {
				w = 1.0f;
			}
			size_t destOffset = i * 4;
			dest[destOffset + 0] = x;
			dest[destOffset + 1] = y;
			dest[destOffset + 2] = z;
			dest[destOffset + 3] = w;
		}
	}

	virtual void SetViscosity(const float viscosity) = 0;
	virtual void SetStiffness(const float stiffness) = 0;
	virtual void SetMaxMotionDistance(const float maxMotionDistance) = 0;
	virtual void SetContactOffset(const float contactOffset) = 0;
	virtual void SetRestOffset(const float restOffset) = 0;
	virtual void SetRestitution(const float restitution) = 0;
	virtual void SetDamping(const float damping) = 0;
	virtual void SetDynamicFriction(const float dynamicFriction) = 0;
	virtual void SetStaticFriction(const float staticFriction) = 0;
	virtual void SetParticleMass(const float particleMass) = 0;
};

struct PhysicsEngineConfiguration {
	uint32_t threadCount;
	float deltaTime;
};

struct PhysicsParticlesStorage {
	glm::vec3 *positions;
	glm::vec3 *velocities;
	uint32_t numParticles;
};

class PhysicsEngine {
protected:
	std::vector<PhysicsActor *> actors;
	const float stepDT;
	float accumulator;
	bool isInitialized;

	virtual PhysicsParticleSystem *CreateParticleSystem(const FluidSimulationProperties &desc, const uint32_t maxParticleCount) = 0;

	virtual void AddActor(PhysicsActor *actor) = 0;
	virtual void RemoveActor(PhysicsActor *actor) = 0;

	virtual PhysicsRigidBody *CreateRigidBody(const PhysicsRigidBody::MotionKind motionKind, const glm::vec3 &pos, const glm::quat &rotation, const PhysicsShape &shape) = 0;

	virtual void Simulate(const float deltaTime) = 0;

	PhysicsEngine(const PhysicsEngineConfiguration &config);
public:
	virtual ~PhysicsEngine();

	static PhysicsEngine *Create(const PhysicsEngineConfiguration &config);

	virtual void Clear();

	void Step(const float deltaTime);

	PhysicsParticleSystem *AddParticleSystem(const FluidSimulationProperties &desc, const uint32_t maxParticleCount);
	void DeleteParticleSystem(PhysicsParticleSystem *particleSystem);
	virtual bool AddParticles(PhysicsParticleSystem *particleSystem, const PhysicsParticlesStorage &storage) = 0;

	PhysicsRigidBody *AddRigidBody(const PhysicsRigidBody::MotionKind motionKind, const glm::vec3 &pos, const glm::quat &rotation, const PhysicsShape &shape);
	void DeleteRigidBody(PhysicsRigidBody *body);

	virtual bool SupportsGPUAcceleration() = 0;
	virtual bool IsGPUAcceleration() = 0;
	virtual void SetGPUAcceleration(const bool value) = 0;
	virtual void SetGravity(const glm::vec3 &gravity) = 0;
};

