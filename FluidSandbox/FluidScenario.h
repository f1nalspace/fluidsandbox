#pragma once

#include <string>
#include <vector>
#include <map>

#include <PxPhysicsAPI.h> 
#include <glm/glm.hpp>

#include "FluidContainer.hpp"
#include "OSLowLevel.h"
#include "XMLUtils.h"
#include "Utils.h"
#include "Actor.h"
#include "CubeActor.h"
#include "SphereActor.h"
#include "Scene.h"
using namespace std;

#include <rapidxml/rapidxml.hpp>
using namespace rapidxml;

class CFluidScenario
{
private:
	string name;
	float viscosity;
	float stiffness;
	float damping;
	float particleDistanceFactor;
	float particleRenderFactor;
	float particleRadius;
	float particleMinDensity;
	vector<CActor*> actors;
	vector<FluidContainer*> fluidContainers;
	PxVec3 actorCreatePosition;
	PxVec3 gravity;
public:
	CFluidScenario();
	~CFluidScenario(void);
	const char* getName() { return name.c_str(); }
	void addActor(CActor* actor) { actors.push_back(actor); }
	void addFluidContainer(FluidContainer* fluidContainer) { fluidContainers.push_back(fluidContainer); }
	int getActors() { return actors.size(); }
	CActor* getActor(const int index) { return actors[index]; }
	int getFluidContainers() { return fluidContainers.size(); }
	FluidContainer* getFluidContainer(int index) { return fluidContainers[index]; }
	static CFluidScenario* load(const char* filename, CScene* scene);
	float getViscosity() { return viscosity; }
	float getStiffness() { return stiffness; }
	float getDamping() { return damping; }
	float getParticleDistanceFactor() { return particleDistanceFactor; }
	float getParticleRenderFactor() { return particleRenderFactor; }
	float getParticleRadius() { return particleRadius; }
	float getParticleMinDensity() { return particleMinDensity; }
	PxVec3 getActorCreatePosition() { return actorCreatePosition; }
	PxVec3 getGravity() { return gravity; }
	void setViscosity(const float value) { viscosity = value; }
	void setStiffness(const float value) { stiffness = value; }
	void setDamping(const float value) { damping = value; }
	void setParticleDistanceFactor(const float value) { particleDistanceFactor = value; }
	void setParticleRenderFactor(const float value) { particleRenderFactor = value; }
	void setParticleRadius(const float value) { particleRadius = value; }
	void setParticleMinDensity(const float value) { particleMinDensity = value; }
	void setActorCreatePosition(PxVec3 pos) { actorCreatePosition = pos; }
	void setName(const char* name) { this->name = name; }
	void setGravity(const PxVec3 value) { gravity = value; }
};

