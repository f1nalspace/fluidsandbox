#include "FluidSystem.h"

CFluidSystem::CFluidSystem(physx::PxPhysics* physics, const FluidSimulationProperties &desc, const uint32_t maxParticles)
{
	this->physics = physics;
	this->maxParticles = maxParticles;
	this->indexPool = physx::PxParticleExt::createIndexPool(maxParticles);
	this->currentParticles = 0;
	this->particleFluid = physics->createParticleFluid(maxParticles);
	this->particleFluid->setParticleReadDataFlag(physx::PxParticleReadDataFlag::ePOSITION_BUFFER, true);
	this->particleFluid->setParticleReadDataFlag(physx::PxParticleReadDataFlag::eDENSITY_BUFFER, true);
	this->particleFluid->setParticleReadDataFlag(physx::PxParticleReadDataFlag::eVELOCITY_BUFFER, true);
	this->particleFluid->setStiffness(desc.stiffness);
	this->particleFluid->setViscosity(desc.viscosity);
	this->particleFluid->setRestitution(desc.restitution);
	this->particleFluid->setDamping(desc.damping);
	this->particleFluid->setDynamicFriction(desc.dynamicFriction);
	this->particleFluid->setMaxMotionDistance(desc.maxMotionDistance);
	this->particleFluid->setRestOffset(desc.restOffset);
	this->particleFluid->setContactOffset(desc.contactOffset);
	this->particleFluid->setRestParticleDistance(desc.restParticleDistance);
	this->particleFluid->setParticleMass(desc.particleMass);
	this->particleFluid->setGridSize(desc.gridSize);
}

CFluidSystem::~CFluidSystem(void)
{
	this->indexPool->freeIndices();
	this->indexPool->release();
	this->particleFluid = NULL;
}

int CFluidSystem::createParticles(const uint32_t numParticles, physx::PxVec3 *pos, physx::PxVec3 *vel)
{
	std::vector<physx::PxU32> indicesList;
	std::vector<physx::PxVec3> positions;
	std::vector<physx::PxVec3> velocities;

	int addedParticles = 0; 
	for (uint32_t i = 0; i < numParticles; i++) {
		if (this->currentParticles < this->maxParticles) {
			positions.push_back(pos[i]);
			velocities.push_back(vel[i]);
			indicesList.push_back(physx::PxU32(0));
			addedParticles++;
			this->currentParticles++;
		}
	}

	physx::PxStrideIterator<physx::PxU32> indexBuffer(&indicesList[0]);
	physx::PxU32 numAllocated = indexPool->allocateIndices(addedParticles, indexBuffer);

	physx::PxParticleCreationData particleCreationData;
	particleCreationData.numParticles = addedParticles;
	particleCreationData.indexBuffer = indexBuffer;
	particleCreationData.positionBuffer = physx::PxStrideIterator<physx::PxVec3>(&positions[0]);
	particleCreationData.velocityBuffer = physx::PxStrideIterator<physx::PxVec3>(&velocities[0]);

	this->particleFluid->createParticles(particleCreationData);

	return addedParticles;
}

physx::PxParticleFluidReadData* CFluidSystem::lockReadData()
{
	return this->particleFluid->lockParticleFluidReadData();
}

void CFluidSystem::setExternalAcceleration(const physx::PxVec3 &acc)
{
	this->particleFluid->setExternalAcceleration(acc);
}

void CFluidSystem::releaseParticles(const physx::PxStrideIterator<physx::PxU32> &indices, const physx::PxU32 count)
{
	this->particleFluid->releaseParticles(count, indices);
	indexPool->freeIndices(count, indices);

	this->currentParticles-=count;
	if (this->currentParticles < 0) this->currentParticles = 0;
}

void CFluidSystem::addForce(const physx::PxVec3 &acc, const physx::PxForceMode::Enum &mode)
{
	// TODO(final): Do not use a std::vector here, use a static array instead!
	std::vector<physx::PxU32> indices;
	std::vector<physx::PxVec3> forces;
	for (uint32_t i = 0; i < currentParticles; i++) {
		indices.push_back(i);
		forces.push_back(physx::PxVec3(acc));
	}
	physx::PxStrideIterator<const physx::PxU32> indexBuffer(&indices[0]);
	physx::PxStrideIterator<const physx::PxVec3> forceBuffer(&forces[0]);
	this->particleFluid->addForces(this->currentParticles, indexBuffer, forceBuffer, mode);
}

void CFluidSystem::writeToVBO(float* data, uint32_t &count, const bool noDensity, const float minDensity)
{
	// TODO(final): Do not use a std::vector here, use a static array instead!
	std::vector<physx::PxU32> deletedPartices;
	count = 0;
	physx::PxParticleFluidReadData* rd = this->particleFluid->lockParticleFluidReadData();
	if (rd)
	{
		int idx = 0;
		physx::PxStrideIterator<const physx::PxParticleFlags> flagsIt(rd->flagsBuffer);
		physx::PxStrideIterator<const physx::PxVec3> positionIt(rd->positionBuffer);
		physx::PxStrideIterator<const physx::PxF32> densityIt(rd->densityBuffer);
		physx::PxStrideIterator<const physx::PxVec3> velocityIt(rd->velocityBuffer);
		for (unsigned i = 0; i < rd->validParticleRange; ++i, ++flagsIt, ++positionIt, ++densityIt, ++velocityIt) 
		{
			bool drain = *flagsIt & physx::PxParticleFlag::eCOLLISION_WITH_DRAIN;
			if (drain) {
				deletedPartices.push_back(i);
			}
			if (*flagsIt & physx::PxParticleFlag::eVALID && !drain)
			{
				data[idx+0] = positionIt->x;
				data[idx+1] = positionIt->y;
				data[idx+2] = positionIt->z;
				if (!noDensity) {
					data[idx+3] = *densityIt;
					if (data[idx+3] > 1.0f) data[idx+3] = 1.0f;
					else if (data[idx+3] < minDensity) data[idx+3] = minDensity;
				} else {
					data[idx+3] = 1.0f;
				}
				idx+=4;
				count++;
			}
		}
		rd->unlock();
	}
	if (deletedPartices.size() > 0) {
		releaseParticles(physx::PxStrideIterator<physx::PxU32>(&deletedPartices[0]), (physx::PxU32)deletedPartices.size());
	}
}