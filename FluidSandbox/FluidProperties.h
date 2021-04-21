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
	constexpr static float DefaultContactOffsetFactor = 2.0f;
	// The factor for computing the restParticleDistance based on the particle radius
	constexpr static float DefaultParticleRestDistanceFactor = 2.0f;
	// The factor how cell sizes are computed based on the particle radius
	constexpr static float DefaultCellSizeFactor = 1.0f;

	// 40-50 is a good stable configuration
	constexpr static float DefaultViscosity = 40.0f;
	constexpr static float DefaultStiffness = 50.0f;

	constexpr static float DefaultRestitution = 0.25f;
	constexpr static float DefaultDamping = 0.001f;
	constexpr static float DefaultDynamicFriction = 0.001f;
	constexpr static float DefaultStaticFriction = 0.0f;
	constexpr static float DefaultMaxMotionDistance = 0.3f;
	constexpr static float DefaultRestOffset = 0.12f;
	constexpr static float DefaultContactOffset = 0.24f;
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
		result.restOffset = DefaultRestOffset;
		result.contactOffset = result.restOffset * DefaultContactOffsetFactor;
		result.particleMass = DefaultParticleMass;

		result.particleRadius = particleRadius;
		result.particleDistanceFactor = particleDistanceFactor;
		result.restParticleDistance = particleRadius * particleDistanceFactor;
		result.cellSize = particleRadius * DefaultCellSizeFactor;

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
		assert(cellSize >= particleRadius);
	}
};

struct FluidRenderProperties {
	constexpr static float DefaultParticleRenderFactor = 1.5f;
	constexpr static float DefaultMinDensity = 0.01f;

	float particleRenderFactor;
	float minDensity;
};