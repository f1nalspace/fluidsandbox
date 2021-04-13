#pragma once

#include "Actor.hpp"

struct CSphereActor: public CActor {
	float radius;

	CSphereActor(const ActorType type):
		CActor(type, ActorPrimitiveKind::Sphere),
		radius(0.5f) {

	}
};