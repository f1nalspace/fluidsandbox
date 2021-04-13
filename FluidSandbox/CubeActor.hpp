#pragma once

#include "Actor.hpp"

struct CCubeActor :	public CActor
{
	physx::PxVec3 size;

	CCubeActor(const ActorType type):
		CActor(type, ActorPrimitiveKind::Cube) {
	}
};

