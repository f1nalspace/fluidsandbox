#pragma once

#include <PxPhysicsAPI.h> 
using namespace physx;

enum EActorType
{
	ActorTypeStatic = 0,
	ActorTypeDynamic
};

enum EActorPrimitive
{
	ActorPrimitiveCube = 0,
	ActorPrimitiveSphere
};

class CActor
{
private:
	EActorType type;
	EActorPrimitive primitive;
	PxVec3 pos;
	int time;
	float timeElapsed;
	PxVec4 color;
	float density;
	PxVec3 velocity;
	PxVec3 rotate;
	bool visible;
	bool blending;
	bool particleDrain;
public:
	CActor(const EActorType type, const EActorPrimitive prim);
	~CActor(void);
	PxVec3 getPos() { return pos; }
	void setPos(const PxVec3 value) { pos = value; }
	PxVec3 getVelocity() { return velocity; }
	void setVelocity(const PxVec3 value) { velocity = value; }
	int getTime() { return time; }
	void setTime(const int time) { this->time = time; }
	float getTimeElapsed() { return timeElapsed; }
	void setTimeElapsed(const float timeElapsed) { this->timeElapsed = timeElapsed; }
	PxVec4 getColor() { return color; }
	void setColor(PxVec4 color) { this->color = color; }
	EActorType getType() { return type; }
	void setPrimitive(EActorPrimitive prim) { primitive = prim; }
	EActorPrimitive getPrimitive() { return primitive; }
	float getDensitiy() { return density; }
	void setDensity(float value) { density = value; }
	bool getVisible() { return visible; }
	void setVisible(bool value) { visible = value; }
	bool getBlending() { return blending; }
	void setBlending(bool value) { blending = value; }
	PxVec3 getRotate() { return rotate; }
	void setRotate(const PxVec3 value) { rotate = value; }
	bool getParticleDrain() { return particleDrain; }
	void setParticleDrain(bool value) { particleDrain = value; }
};
