#pragma once

constexpr float DefaultFluidViscosity = 10.0f;
constexpr float DefaultFluidStiffness = 50.0f;
constexpr float DefaultFluidRestitution = 0.3f;
constexpr float DefaultFluidDamping = 0.001f;
constexpr float DefaultFluidDynamicFriction = 0.001f;
constexpr float DefaultFluidMaxMotionDistance = 0.3f;
constexpr float DefaultFluidRestOffset = 0.3f; // 0.12
constexpr float DefaultFluidContactOffset = 2.0f; // 0.036f;
constexpr float DefaultFluidParticleRestDistanceFactor = 2.0f;
constexpr float DefaultFluidParticleMass = 0.005f;
constexpr float DefaultFluidParticleGridFactor = 6.0f;

#if 0
#endif

struct FluidSimulationProperties {
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