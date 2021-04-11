#ifndef LIGHT_H
#define LIGHT_H

#include <PxPhysicsAPI.h>
using namespace physx;

namespace LightType {
	enum Type {
		Point,
		Directional
	};
};

struct Light {
	LightType::Type type;
	PxVec3 pos;
	float radius;
	PxVec4 color;
	float falloff;
	float shininess;
	PxVec3 dir;
	float accel;
	float curAngle;
	inline Light() {
		type = LightType::Point;
		pos = PxVec3(0.0);
		radius = 0.0f;
		color = PxVec4(0.0f);
		falloff = 0.0f;
		shininess = 10.0f;
		dir = PxVec3(0.0f);
		accel = 0.0f;
		curAngle = 0.0f;
	};
	inline Light(const LightType::Type &type, const PxVec3 &pos, const float &radius, const PxVec4 &color, const float &falloff, const float &shininess) {
		this->type = type;
		this->pos = pos;
		this->radius = radius;
		this->color = color;
		this->falloff = falloff;
		this->shininess = shininess;
		this->dir = PxVec3(0.0f);
		this->accel = 0.0f;
		this->curAngle = 0.0f;
	};
};

#endif