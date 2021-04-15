#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum class ActorMovementType {
	Static = 0,
	Dynamic
};

enum class ActorType {
	None = 0,
	Plane,
	Cube,
	Sphere,
	Capsule,
	Fluid
};

struct ActorTransform {
	glm::quat rotation;
	glm::vec3 position;

	ActorTransform():
		rotation(glm::quat()),
		position(glm::vec3(0.0f)) {

	}
};

struct Actor {
	ActorTransform transform;
	glm::vec4 color;
	glm::vec3 velocity;
	void *physicsData;
	float timeElapsed;
	float density;
	ActorType type;
	ActorMovementType movementType;
	int time;
	bool visible;
	bool blending;
	bool particleDrain;

	Actor(const ActorType type, const ActorMovementType movementType):
		transform(ActorTransform()),
		color(glm::vec4(1.0f)),
		velocity(glm::vec3(0.0f)),
		physicsData(nullptr),
		timeElapsed(0.0f),
		density(1.0f),
		type(type),
		movementType(movementType),
		time(0),
		visible(true),
		blending(false),
		particleDrain(false) {
	}
};
