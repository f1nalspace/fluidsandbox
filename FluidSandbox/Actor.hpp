#pragma once

#include <glm/glm.hpp>

enum class ActorType
{
	ActorTypeStatic = 0,
	ActorTypeDynamic
};

enum class ActorPrimitiveKind
{
	Cube = 0,
	Sphere
};

struct CActor
{
	glm::vec4 color;
	glm::vec3 pos;
	glm::vec3 velocity;
	glm::vec3 rotate;
	float timeElapsed;
	float density;
	ActorType type;
	ActorPrimitiveKind primitive;
	int time;
	bool visible;
	bool blending;
	bool particleDrain;

	CActor(const ActorType type, const ActorPrimitiveKind prim) {
		this->type = type;
		this->time = 0;
		this->timeElapsed = 0.0f;
		this->pos = glm::vec3(0);
		this->color = glm::vec4(1.0f);
		this->primitive = prim;
		this->density = 1.0f;
		this->velocity = glm::vec3(0.0f);
		this->visible = true;
		this->blending = false;
		this->rotate = glm::vec3(0.0f);
		this->particleDrain = false;
	}
};
