#ifndef LIGHT_H
#define LIGHT_H

#include <PxPhysicsAPI.h>

enum class LightType {
	Point,
	Directional
};

struct Light {
	physx::PxVec4 color;
	physx::PxVec3 pos;
	physx::PxVec3 dir;
	float radius;
	float falloff;
	float shininess;
	float accel;
	float curAngle;
	LightType type;

	inline Light() {
		type = LightType::Point;
		pos = physx::PxVec3(0.0);
		radius = 0.0f;
		color = physx::PxVec4(0.0f);
		falloff = 0.0f;
		shininess = 10.0f;
		dir = physx::PxVec3(0.0f);
		accel = 0.0f;
		curAngle = 0.0f;
	};

	inline Light(const LightType &type, const physx::PxVec3 &pos, const float &radius, const physx::PxVec4 &color, const float &falloff, const float &shininess) {
		this->type = type;
		this->pos = pos;
		this->radius = radius;
		this->color = color;
		this->falloff = falloff;
		this->shininess = shininess;
		this->dir = physx::PxVec3(0.0f);
		this->accel = 0.0f;
		this->curAngle = 0.0f;
	};
};

#endif