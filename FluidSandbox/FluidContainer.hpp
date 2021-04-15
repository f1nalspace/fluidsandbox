#pragma once

#include <glm/glm.hpp> 

enum class FluidType: int {
	None = -1,
	Drop = 0,
	Plane = 1,
	Box = 2,
	Sphere = 3,
};

struct FluidContainer {
	glm::vec3 position;
	glm::vec3 size;
	glm::vec3 velocity;
	float radius;
	float timeElapsed;
	float emitterRate;
	float emitterTime;
	float emitterElapsed;
	float emitterCoolDownElapsed;
	uint32_t emitterCoolDown;
	uint32_t emitterDuration;
	int time;
	FluidType type;
	bool isEmitter;
	bool emitterCoolDownActive;

	FluidContainer(const glm::vec3 &position, const glm::vec3 &size, const FluidType type):
		position(position),
		size(size),
		type(type),
		velocity(glm::vec3(0.0f)),
		time(0),
		timeElapsed(0),
		radius(0),
		isEmitter(false),
		emitterCoolDown(0),
		emitterRate(0),
		emitterTime(0),
		emitterDuration(0),
		emitterElapsed(0),
		emitterCoolDownActive(false),
		emitterCoolDownElapsed(0) {
	}
};
