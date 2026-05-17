/*
======================================================================================================================
	Fluid Sandbox - FluidProperties.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <assert.h>

struct FluidSimulationProperties {
	// The factor for computing the contactOffset based on the restOffset.
	// Must be > 1 so the contact offset stays strictly larger than the rest offset.
	constexpr static float DefaultContactOffsetFactor = 1.5f;
	// The factor for computing the restParticleDistance based on the particle radius
	constexpr static float DefaultParticleRestDistanceFactor = 2.0f;
	// Safety multiplier applied to the minimum viable broadphase grid cell size.
	// 1.0 = exactly the minimum; larger trades memory for a wider safety margin.
	constexpr static float DefaultCellSizeFactor = 1.0f;

	// 40-50 is a good stable configuration
	constexpr static float DefaultViscosity = 40.0f;
	constexpr static float DefaultStiffness = 50.0f;

	// Water does not bounce - keep restitution low for a water-like, stable fluid
	constexpr static float DefaultRestitution = 0.1f;
	constexpr static float DefaultDamping = 0.001f;
	constexpr static float DefaultDynamicFriction = 0.001f;
	constexpr static float DefaultStaticFriction = 0.0f;
	// Must stay below the grid cell size so a particle can never skip a cell
	constexpr static float DefaultMaxMotionDistance = 0.1f;
	// Particle-vs-rigidbody rest distance. Must stay <= restParticleDistance,
	// a particle radius is a good value.
	constexpr static float DefaultRestOffset = 0.05f;
	constexpr static float DefaultContactOffset = DefaultRestOffset * DefaultContactOffsetFactor;
	constexpr static float DefaultParticleMass = 0.005f;
	constexpr static float DefaultParticleRadius = 0.05f;

	float viscosity;				// Thickness [5-300]
	float stiffness;				// Gas-constant [1-200], 1 = Compressed, 200 = Less compressed, more unstable
	float restitution;				// How fluids bounces of surfaces [0-1]
	float damping;					// Destroying of energy to increase stability
	float dynamicFriction;			// Friction used when interacting with rigid bodies when in motion [0-1]
	float staticFriction;			// Friction used when interacting with rigid bodies when in rest [0-1]
	float maxMotionDistance;		// Much much particles can move for one timestep
	float restOffset;				// Distance of particles how interacting with with each other
	float contactOffset;			// Smallest distance a collision contact gets generated
	float particleMass;				// The mass of the particle
	float particleRadius;			// The radius of the particle, used for computing the grid size and the rest particle distance

	// TODO(final): Remove the particle distance factor!
	float particleDistanceFactor;	// The distance factor used for multiplying the radius with. Used for create particles which touches each other.

	float restParticleDistance;		// The ideal distance when particles touch each other
	float cellSize;					// The size of a one grid cell, must be equal or greater than the particle radius (h)

	static FluidSimulationProperties Compute(const float particleRadius, const float particleDistanceFactor) {
		FluidSimulationProperties result = {};
		result.viscosity = DefaultViscosity;
		result.stiffness = DefaultStiffness;
		result.restitution = DefaultRestitution;
		result.damping = DefaultDamping;
		result.dynamicFriction = DefaultDynamicFriction;
		result.staticFriction = DefaultStaticFriction;
		result.maxMotionDistance = DefaultMaxMotionDistance;
		result.particleMass = DefaultParticleMass;

		result.particleRadius = particleRadius;
		result.particleDistanceFactor = particleDistanceFactor;
		result.restParticleDistance = particleRadius * particleDistanceFactor;

		// The rest offset is a particle-vs-rigidbody distance and must stay below
		// the inter-particle rest distance, otherwise particles get over-compressed
		// against walls which causes pressure spikes (corner explosions).
		result.restOffset = particleRadius;
		result.contactOffset = result.restOffset * DefaultContactOffsetFactor;

		// The broadphase grid cell must be large enough to contain the full SPH
		// interaction radius (~2 * restParticleDistance) as well as the worst-case
		// per-step displacement plus the rigidbody collision band. If it is smaller,
		// neighbour and collision queries silently miss particles, which makes the
		// SPH density/pressure asymmetric and blows the simulation up - worst in
		// corners where the density is already high.
		const float interactionRadius = 2.0f * result.restParticleDistance;
		const float collisionReach = result.maxMotionDistance + 2.0f * result.contactOffset;
		const float minCellSize = interactionRadius > collisionReach ? interactionRadius : collisionReach;
		result.cellSize = minCellSize * DefaultCellSizeFactor;

		result.Validate();

		return(result);
	}

	void Validate() {
		// Range validation
		assert(particleRadius > 0);
		assert(restitution >= 0 && restitution <= 1.0f);
		assert(dynamicFriction >= 0 && dynamicFriction <= 1.0f);
		assert(staticFriction >= 0 && staticFriction <= 1.0f);

		// PhysX restrictions
		assert(viscosity >= 5 && viscosity <= 300.0f);
		assert(stiffness >= 1 && stiffness <= 200.0f);
		assert(contactOffset >= restOffset);
		assert(restParticleDistance >= 0.05f);

		// Stability relationships - violating any of these makes the SPH solver
		// miss neighbours/collisions and the fluid becomes unstable.
		assert(restOffset <= restParticleDistance);        // avoid over-compression at walls
		assert(cellSize >= 2.0f * restParticleDistance);   // grid must cover the SPH kernel
		assert(maxMotionDistance < cellSize);              // a particle may not skip a grid cell
	}

	// Repairs any out-of-range or inconsistent value back to a safe default and
	// recomputes derived values. Use after loading values from untrusted XML so
	// a bad parameter falls back to a default instead of breaking the solver.
	void Sanitize() {
		if(!(particleRadius > 0.0f)) particleRadius = DefaultParticleRadius;
		if(!(particleDistanceFactor > 0.0f)) particleDistanceFactor = DefaultParticleRestDistanceFactor;
		restParticleDistance = particleRadius * particleDistanceFactor;

		if(viscosity < 5.0f || viscosity > 300.0f) viscosity = DefaultViscosity;
		if(stiffness < 1.0f || stiffness > 200.0f) stiffness = DefaultStiffness;
		if(restitution < 0.0f || restitution > 1.0f) restitution = DefaultRestitution;
		if(damping < 0.0f || damping > 1.0f) damping = DefaultDamping;
		if(dynamicFriction < 0.0f || dynamicFriction > 1.0f) dynamicFriction = DefaultDynamicFriction;
		if(staticFriction < 0.0f || staticFriction > 1.0f) staticFriction = DefaultStaticFriction;
		if(!(particleMass > 0.0f)) particleMass = DefaultParticleMass;
		if(!(maxMotionDistance > 0.0f)) maxMotionDistance = DefaultMaxMotionDistance;

		// restOffset must be > 0 and not exceed the inter-particle rest distance
		if(!(restOffset > 0.0f) || restOffset > restParticleDistance)
			restOffset = particleRadius < restParticleDistance ? particleRadius : restParticleDistance;
		// contactOffset must be strictly larger than restOffset
		if(contactOffset <= restOffset)
			contactOffset = restOffset * DefaultContactOffsetFactor;

		// Recompute the grid cell size from the (possibly overridden) values so it
		// always covers the SPH kernel and the per-step collision reach.
		const float interactionRadius = 2.0f * restParticleDistance;
		const float collisionReach = maxMotionDistance + 2.0f * contactOffset;
		const float minCellSize = interactionRadius > collisionReach ? interactionRadius : collisionReach;
		cellSize = minCellSize * DefaultCellSizeFactor;

		Validate();
	}
};

struct FluidRenderProperties {
	constexpr static float DefaultParticleRenderFactor = 1.5f;
	constexpr static float DefaultMinDensity = 0.01f;

	float particleRenderFactor;
	float minDensity;
};