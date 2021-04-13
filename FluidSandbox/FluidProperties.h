#pragma once

struct FluidSimulationProperties {
	constexpr static float DefaultViscosity = 10.0f;
	constexpr static float DefaultStiffness = 50.0f;
	constexpr static float DefaultRestitution = 0.3f;
	constexpr static float DefaultDamping = 0.001f;
	constexpr static float DefaultDynamicFriction = 0.001f;
	constexpr static float DefaultMaxMotionDistance = 0.3f;
	constexpr static float DefaultRestOffset = 0.3f; // 0.12
	constexpr static float DefaultContactOffset = 2.0f; // 0.036f;
	constexpr static float DefaultParticleRestDistanceFactor = 2.0f;
	constexpr static float DefaultParticleMass = 0.005f;
	constexpr static float DefaultParticleGridFactor = 6.0f;

	float viscosity;
	float stiffness;
	float restitution;
	float damping;
	float dynamicFriction;
	float maxMotionDistance;
	float restOffset;
	float contactOffset;
	float restParticleDistance;
	float particleMass;
	float gridSize;
};