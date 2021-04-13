#pragma once

#include <glm/glm.hpp> 

enum class FluidType {
	Drop = 0,
	Plane = 1,
	Box = 2,
	Sphere = 3,
};

struct FluidContainer
{
	glm::vec3 pos;
	glm::vec3 size;
	glm::vec3 vel;
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

	FluidContainer(const glm::vec3 &pos, const glm::vec3 &size, const FluidType type){
		this->pos = pos;
		this->size = size;
		this->type = type;
		vel = glm::vec3(0.0f);
		time = 0;
		timeElapsed = 0.0f;
		radius = 0.0f;
		isEmitter = false;
		emitterCoolDown = 0;
		emitterRate = 0.0f;
		emitterTime = 0.0f;
		emitterDuration = 0;
		emitterElapsed = 0.0f;
		emitterCoolDownActive = false;
		emitterCoolDownElapsed = 0.0f;
	}
};
