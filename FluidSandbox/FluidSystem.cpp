#include "FluidSystem.h"

CFluidSystem::CFluidSystem(PxPhysics* physics, FluidDescription &desc, const unsigned int maxParticles)
{
	this->physics = physics;
	this->maxParticles = maxParticles;
	this->indexPool = PxParticleExt::createIndexPool(maxParticles);
	this->currentParticles = 0;
	this->particleFluid = physics->createParticleFluid(desc.maxParticles);
	this->particleFluid->setParticleReadDataFlag(PxParticleReadDataFlag::ePOSITION_BUFFER, true);
	this->particleFluid->setParticleReadDataFlag(PxParticleReadDataFlag::eDENSITY_BUFFER, true);
	this->particleFluid->setParticleReadDataFlag(PxParticleReadDataFlag::eVELOCITY_BUFFER, true);
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

int CFluidSystem::createParticles(const unsigned int numParticles, PxVec3 *pos, PxVec3 *vel)
{
	vector<PxU32> indicesList;
	vector<PxVec3> positions;
	vector<PxVec3> velocities;

	int addedParticles = 0; 
	for (unsigned int i = 0; i < numParticles; i++) {
		if (this->currentParticles < this->maxParticles) {
			positions.push_back(pos[i]);
			velocities.push_back(vel[i]);
			indicesList.push_back(PxU32(0));
			addedParticles++;
			this->currentParticles++;
		}
	}

	PxStrideIterator<PxU32> indexBuffer(&indicesList[0]);
	PxU32 numAllocated = indexPool->allocateIndices(addedParticles, indexBuffer);

	PxParticleCreationData particleCreationData;
	particleCreationData.numParticles = addedParticles;
	particleCreationData.indexBuffer = indexBuffer;
	particleCreationData.positionBuffer = PxStrideIterator<PxVec3>(&positions[0]);
	particleCreationData.velocityBuffer = PxStrideIterator<PxVec3>(&velocities[0]);

	this->particleFluid->createParticles(particleCreationData);

	return addedParticles;
}

PxParticleFluidReadData* CFluidSystem::lockReadData()
{
	return this->particleFluid->lockParticleFluidReadData();
}

void CFluidSystem::setExternalAcceleration(const PxVec3 &acc)
{
	this->particleFluid->setExternalAcceleration(acc);
}

void CFluidSystem::releaseParticles(const PxStrideIterator<PxU32> &indices, const PxU32 count)
{
	this->particleFluid->releaseParticles(count, indices);
	indexPool->freeIndices(count, indices);

	this->currentParticles-=count;
	if (this->currentParticles < 0) this->currentParticles = 0;
}

void CFluidSystem::addForce(const PxVec3 &acc, const PxForceMode::Enum &mode)
{
	vector<PxU32> indices;
	for (unsigned int i = 0; i < currentParticles; i++) {
		indices.push_back(i);
	}
	PxStrideIterator<const PxU32> indexBuffer(&indices[0]);
	vector<PxVec3> forces;
	for (unsigned int i = 0; i < currentParticles; i++) {
		forces.push_back(PxVec3(acc));
	}
	PxStrideIterator<const PxVec3> forceBuffer(&forces[0]);
	this->particleFluid->addForces(this->currentParticles, indexBuffer, forceBuffer, mode);
}

void CFluidSystem::writeToVBO(float* data, unsigned int &count, const bool noDensity, const float minDensity)
{
	vector<PxU32> deletedPartices;
	count = 0;
	PxParticleFluidReadData* rd = this->particleFluid->lockParticleFluidReadData();
	if (rd)
	{
		int idx = 0;
		PxStrideIterator<const PxParticleFlags> flagsIt(rd->flagsBuffer);
		PxStrideIterator<const PxVec3> positionIt(rd->positionBuffer);
		PxStrideIterator<const PxF32> densityIt(rd->densityBuffer);
		PxStrideIterator<const PxVec3> velocityIt(rd->velocityBuffer);
		for (unsigned i = 0; i < rd->validParticleRange; ++i, ++flagsIt, ++positionIt, ++densityIt, ++velocityIt) 
		{
			bool drain = *flagsIt & PxParticleFlag::eCOLLISION_WITH_DRAIN;
			if (drain) {
				deletedPartices.push_back(i);
			}
			if (*flagsIt & PxParticleFlag::eVALID && !drain)
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
		releaseParticles(PxStrideIterator<PxU32>(&deletedPartices[0]), (PxU32)deletedPartices.size());
	}
}