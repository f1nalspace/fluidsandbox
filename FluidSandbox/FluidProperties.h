#pragma once

struct FluidSimulationProperties {
	constexpr static float DefaultViscosity = 10.0f;
	constexpr static float DefaultStiffness = 50.0f;
	constexpr static float DefaultRestitution = 0.3f;
	constexpr static float DefaultDamping = 0.001f;
	constexpr static float DefaultDynamicFriction = 0.001f;
	constexpr static float DefaultMaxMotionDistance = 0.3f;
	constexpr static float DefaultRestOffset = 0.12f;
	constexpr static float DefaultContactOffset = 0.036f;
	constexpr static float DefaultParticleMass = 0.005f;
	constexpr static float DefaultParticleRadius = 0.05f;
	constexpr static float DefaultParticleRestDistanceFactor = 2.0f;
	constexpr static float DefaultParticleGridFactor = 6.0f;

	float viscosity;
	float stiffness;
	float restitution;
	float damping;
	float dynamicFriction;
	float maxMotionDistance;
	float restOffset;
	float contactOffset;
	float particleMass;
	float particleRadius;
	float particleDistanceFactor;
	float restParticleDistance;
	float gridSize;

	static FluidSimulationProperties Compute(const float particleRadius, const float particleDistanceFactor) {
		FluidSimulationProperties result = {};
		result.viscosity = DefaultViscosity;
		result.stiffness = DefaultStiffness;
		result.restitution = DefaultRestitution;
		result.damping = DefaultDamping;
		result.dynamicFriction = DefaultDynamicFriction;
		result.maxMotionDistance = DefaultMaxMotionDistance;
		result.restOffset = DefaultRestOffset;
		result.contactOffset = DefaultContactOffset;
		result.particleMass = DefaultParticleMass;

		result.particleRadius = particleRadius;
		result.particleDistanceFactor = particleDistanceFactor;
		result.restParticleDistance = particleRadius * particleDistanceFactor;
		result.gridSize = particleRadius * DefaultParticleGridFactor;
		return(result);
	}
};

struct FluidRenderProperties {
	constexpr static float DefaultParticleRenderFactor = 1.5f;
	constexpr static float DefaultMinDensity = 0.01f;

	float particleRenderFactor;
	float minDensity;
};