#pragma once

struct FluidDescription {
	int maxParticles;
	float stiffness;
	float viscosity;
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