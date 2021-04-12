#ifndef LIGHT_H
#define LIGHT_H

#include <PxPhysicsAPI.h>

struct Light {
	enum class Type {
		Point,
		Directional
	};

	Type type;
	physx::PxVec3 pos;
	float radius;
	physx::PxVec4 color;
	float falloff;
	float shininess;
	physx::PxVec3 dir;
	float accel;
	float curAngle;
	inline Light() {
		type = Type::Point;
		pos = physx::PxVec3(0.0);
		radius = 0.0f;
		color = physx::PxVec4(0.0f);
		falloff = 0.0f;
		shininess = 10.0f;
		dir = physx::PxVec3(0.0f);
		accel = 0.0f;
		curAngle = 0.0f;
	};
	inline Light(const Type &type, const physx::PxVec3 &pos, const float &radius, const physx::PxVec4 &color, const float &falloff, const float &shininess) {
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