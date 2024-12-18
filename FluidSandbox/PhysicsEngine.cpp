/*
======================================================================================================================
	Fluid Sandbox - PhysicsEngine.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "PhysicsEngine.h"

//#define PVD_ENABLED

// PhysX API
#include <PxPhysicsAPI.h>

#include <iostream>
#include <algorithm>
#include <typeinfo>

#include "OSLowLevel.h"

namespace PhysicsUtils {
	inline physx::PxForceMode::Enum toPxForceMode(const PhysicsForceMode mode) {
		switch(mode) {
			case PhysicsForceMode::Acceleration:
				return physx::PxForceMode::eACCELERATION;
			case PhysicsForceMode::VelocityChange:
				return physx::PxForceMode::eVELOCITY_CHANGE;
			case PhysicsForceMode::Impulse:
				return physx::PxForceMode::eIMPULSE;
		}
		return physx::PxForceMode::eFORCE;
	}

	inline physx::PxVec3 toPxVec3(const glm::vec3 &input) {
		return physx::PxVec3(input.x, input.y, input.z);
	}
	inline physx::PxVec4 toPxVec4(const glm::vec4 &input) {
		return physx::PxVec4(input.x, input.y, input.z, input.w);
	}
	inline physx::PxQuat toPxQuat(const glm::quat &input) {
		return physx::PxQuat(input.x, input.y, input.z, input.w);
	}

	inline glm::vec3 toGLMVec3(const physx::PxVec3 &input) {
		return glm::vec3(input.x, input.y, input.z);
	}
	inline glm::vec4 toGLMVec4(const physx::PxVec4 &input) {
		return glm::vec4(input.x, input.y, input.z, input.w);
	}
	inline glm::quat toGLMQuat(const physx::PxQuat &input) {
		return glm::quat(input.w, input.x, input.y, input.z);
	}
}

static physx::PxShape *CreateShape(physx::PxPhysics &physics, physx::PxMaterial &defaultMaterial, const PhysicsShape &shape) {
	physx::PxGeometry *geometry = nullptr;
	physx::PxBoxGeometry boxGeometry;
	physx::PxSphereGeometry sphereGeometry;
	physx::PxCapsuleGeometry capsuleGeometry;
	physx::PxPlaneGeometry planeGeometry;
	switch(shape.type) {
		case PhysicsShape::Type::Box:
			boxGeometry = physx::PxBoxGeometry(shape.box.halfExtents.x, shape.box.halfExtents.y, shape.box.halfExtents.z);
			geometry = &boxGeometry;
			break;
		case PhysicsShape::Type::Sphere:
			sphereGeometry = physx::PxSphereGeometry(shape.sphere.radius);
			geometry = &sphereGeometry;
			break;
		case PhysicsShape::Type::Capsule:
			capsuleGeometry = physx::PxCapsuleGeometry(shape.capsule.radius, shape.capsule.halfHeight);
			geometry = &capsuleGeometry;
			break;
		case PhysicsShape::Type::Plane:
			planeGeometry = physx::PxPlaneGeometry();
			geometry = &planeGeometry;
			break;
		default:
			assert(!"Shape type not supported!");
			break;
	}

	physx::PxShapeFlags shapeFlags =
		physx::PxShapeFlag::eVISUALIZATION |
		physx::PxShapeFlag::eSCENE_QUERY_SHAPE |
		physx::PxShapeFlag::eSIMULATION_SHAPE;
	if(shape.isParticleDrain) {
		shapeFlags |= physx::PxShapeFlag::ePARTICLE_DRAIN;
	}

	physx::PxShape *result = physics.createShape(*geometry, defaultMaterial, true, shapeFlags);
	return(result);
}

struct NativeRigidBody: public PhysicsRigidBody {
	physx::PxRigidActor *actor;
	physx::PxScene *scene;
	physx::PxPhysics *physics;
	physx::PxMaterial *defaultMaterial;

	NativeRigidBody(physx::PxPhysics *physics, physx::PxScene *scene, physx::PxMaterial *defaultMaterial, const MotionKind motionKind, const glm::vec3 &pos, const glm::quat &rotation, const PhysicsShape &shape):
		PhysicsRigidBody(motionKind),
		actor(nullptr),
		physics(physics),
		scene(scene),
		defaultMaterial(defaultMaterial) {

		shapeCount = 1;
		shapes[0] = shape;

		this->transform = PhysicsTransform(pos, rotation);

		physx::PxVec3 npos = PhysicsUtils::toPxVec3(pos);
		physx::PxQuat nrot = PhysicsUtils::toPxQuat(rotation);
		physx::PxTransform ntransform = physx::PxTransform(npos, nrot);

		if(shape.type == PhysicsShape::Type::Plane) {
			npos = physx::PxVec3(0.0f, 0, 0.0f);
			nrot = physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f));
			ntransform = physx::PxTransform(npos, nrot);
			this->actor = physx::PxCreateStatic(*physics, ntransform, physx::PxPlaneGeometry(), *defaultMaterial);
		} else {
			physx::PxShape *newShape = CreateShape(*physics, *defaultMaterial, shape);

			if(motionKind == PhysicsRigidBody::MotionKind::Static) {
				this->actor = physx::PxCreateStatic(*physics, ntransform, *newShape);
			} else {
				this->actor = physx::PxCreateDynamic(*physics, ntransform, *newShape, density);
			}
		}
	}

	~NativeRigidBody() {
		if(actor != nullptr) {
			actor->release();
			actor = nullptr;
		}
	}

	void AddShape(const PhysicsShape &shape) {
		PhysicsRigidBody::AddShape(shape);
		physx::PxShape *newShape = CreateShape(*physics, *defaultMaterial, shape);
		if(newShape != nullptr) {
			actor->attachShape(*newShape);
		}
	}
};

struct NativeParticleSystem: public PhysicsParticleSystem {
	physx::PxParticleExt::IndexPool *indexPool;
	physx::PxParticleFluid *fluid;

	NativeParticleSystem(physx::PxPhysics *physics, const bool useGPUAcceleration, const FluidSimulationProperties &desc, const uint32_t maxParticleCount):
		PhysicsParticleSystem(maxParticleCount),
		indexPool(nullptr),
		fluid(nullptr) {

		indexPool = physx::PxParticleExt::createIndexPool(maxParticleCount);

		fluid = physics->createParticleFluid(maxParticleCount);
		if(fluid != nullptr) {
			fluid->setParticleReadDataFlag(physx::PxParticleReadDataFlag::ePOSITION_BUFFER, true);
			fluid->setParticleReadDataFlag(physx::PxParticleReadDataFlag::eDENSITY_BUFFER, true);
			fluid->setParticleReadDataFlag(physx::PxParticleReadDataFlag::eVELOCITY_BUFFER, true);
			fluid->setStiffness(desc.stiffness);
			fluid->setViscosity(desc.viscosity);
			fluid->setRestitution(desc.restitution);
			fluid->setDamping(desc.damping);
			fluid->setDynamicFriction(desc.dynamicFriction);
			fluid->setStaticFriction(desc.staticFriction);
			fluid->setMaxMotionDistance(desc.maxMotionDistance);
			fluid->setRestOffset(desc.restOffset);
			fluid->setContactOffset(desc.contactOffset);
			fluid->setRestParticleDistance(desc.restParticleDistance);
			fluid->setParticleMass(desc.particleMass);
			fluid->setGridSize(desc.cellSize);

			fluid->setParticleBaseFlag(physx::PxParticleBaseFlag::eCOLLISION_TWOWAY, true);

			// Set GPU acceleration for particle fluid if supported
			fluid->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, useGPUAcceleration);
		}
	}

	~NativeParticleSystem() {
		if(fluid != nullptr) {
			fluid->release();
			fluid = nullptr;
		}
		if(indexPool != nullptr) {
			indexPool->freeIndices();
			indexPool->release();
			indexPool = nullptr;
		}
	}

	void releaseParticles(const physx::PxStrideIterator<physx::PxU32> &indices, const physx::PxU32 count) {
		fluid->releaseParticles(count, indices);
		indexPool->freeIndices(count, indices);

		assert(activeParticleCount >= count);
		activeParticleCount -= count;
	}

	void Syncronize() {
		physx::PxBounds3 nbounds = fluid->getWorldBounds();
		this->bounds = PhysicsBoundingBox(PhysicsUtils::toGLMVec3(nbounds.minimum), PhysicsUtils::toGLMVec3(nbounds.maximum));

		std::vector<physx::PxU32> deletedPartices;

		physx::PxParticleFluidReadData *rd = fluid->lockParticleFluidReadData(physx::PxDataAccessFlag::eREADABLE);
		uint32_t count = 0;
		if(rd != nullptr) {
			physx::PxStrideIterator<const physx::PxParticleFlags> flagsIt(rd->flagsBuffer);
			physx::PxStrideIterator<const physx::PxVec3> positionIt(rd->positionBuffer);
			physx::PxStrideIterator<const physx::PxF32> densityIt(rd->densityBuffer);
			physx::PxStrideIterator<const physx::PxVec3> velocityIt(rd->velocityBuffer);
			for(physx::PxU32 i = 0; i < rd->validParticleRange; ++i, ++flagsIt, ++positionIt, ++velocityIt, ++densityIt) {
				physx::PxParticleFlags flags = *flagsIt;
				bool isDrained = flags & physx::PxParticleFlag::eCOLLISION_WITH_DRAIN;
				if(isDrained) {
					deletedPartices.push_back(i);
				}
				if(flags & physx::PxParticleFlag::eVALID && !isDrained) {
					positions[count].x = positionIt->x;
					positions[count].y = positionIt->y;
					positions[count].z = positionIt->z;
					velocities[count].x = velocityIt->x;
					velocities[count].y = velocityIt->y;
					velocities[count].z = velocityIt->z;
					densities[count] = *densityIt;
					++count;
				}
			}
			rd->unlock();
		}
		if(deletedPartices.size() > 0) {
			releaseParticles(physx::PxStrideIterator<physx::PxU32>(&deletedPartices[0]), (physx::PxU32)deletedPartices.size());
		}
		activeParticleCount = count;
	}

	bool AddParticles(const PhysicsParticlesStorage &storage) {
		assert(storage.positions != nullptr);
		assert(storage.velocities != nullptr);

		std::vector<physx::PxU32> indicesList;
		std::vector<physx::PxVec3> positions;
		std::vector<physx::PxVec3> velocities;

		assert((activeParticleCount + storage.numParticles) <= maxParticleCount);

		int addedParticles = 0;
		for(uint32_t i = 0; i < storage.numParticles; i++) {
			positions.push_back(PhysicsUtils::toPxVec3(storage.positions[i]));
			velocities.push_back(PhysicsUtils::toPxVec3(storage.velocities[i]));
			indicesList.push_back(physx::PxU32(0));
			++addedParticles;
		}
		activeParticleCount += storage.numParticles;

		physx::PxStrideIterator<physx::PxU32> indexBuffer(&indicesList[0]);
		physx::PxU32 numAllocated = indexPool->allocateIndices(addedParticles, indexBuffer);

		physx::PxParticleCreationData particleCreationData;
		particleCreationData.numParticles = addedParticles;
		particleCreationData.indexBuffer = indexBuffer;
		particleCreationData.positionBuffer = physx::PxStrideIterator<physx::PxVec3>(&positions[0]);
		particleCreationData.velocityBuffer = physx::PxStrideIterator<physx::PxVec3>(&velocities[0]);

		bool result = fluid->createParticles(particleCreationData);
		return(result);
	}

	void AddForce(const glm::vec3 &force, const PhysicsForceMode mode) {
		// TODO(final): Do not use a std::vector here, use a static array instead!
		std::vector<physx::PxU32> indices;
		std::vector<physx::PxVec3> forces;
		physx::PxVec3 nforce = PhysicsUtils::toPxVec3(force);
		for(uint32_t i = 0; i < activeParticleCount; i++) {
			indices.push_back(i);
			forces.push_back(nforce);
		}
		physx::PxStrideIterator<const physx::PxU32> indexBuffer(&indices[0]);
		physx::PxStrideIterator<const physx::PxVec3> forceBuffer(&forces[0]);

		physx::PxForceMode::Enum forceMode = PhysicsUtils::toPxForceMode(mode);

		fluid->addForces(activeParticleCount, indexBuffer, forceBuffer, forceMode);
	}

	void SetExternalAcceleration(const glm::vec3 &accel) {
		physx::PxVec3 nacc = PhysicsUtils::toPxVec3(accel);
		fluid->setExternalAcceleration(nacc);
	}

	void SetViscosity(const float viscosity) {
		fluid->setViscosity(viscosity);
	}

	void SetStiffness(const float stiffness) {
		fluid->setStiffness(stiffness);
	}

	void SetMaxMotionDistance(const float maxMotionDistance) {
		fluid->setMaxMotionDistance(maxMotionDistance);
	}

	void SetContactOffset(const float contactOffset) {
		fluid->setContactOffset(contactOffset);
	}

	void SetRestOffset(const float restOffset) {
		fluid->setRestOffset(restOffset);
	}

	void SetRestitution(const float restitution) {
		fluid->setRestitution(restitution);
	}

	void SetDamping(const float damping) {
		fluid->setDamping(damping);
	}

	void SetDynamicFriction(const float dynamicFriction) {
		fluid->setDynamicFriction(dynamicFriction);
	}

	void SetStaticFriction(const float staticFriction) {
		fluid->setStaticFriction(staticFriction);
	}

	void SetParticleMass(const float particleMass) {
		fluid->setParticleMass(particleMass);
	}
};

class NativePhysicsEngine: public PhysicsEngine {
public:
	constexpr static char *PVD_Host = "localhost";
	constexpr static int PVD_Port = 5425;

	physx::PxDefaultErrorCallback defaultErrorCallback;
	physx::PxDefaultAllocator defaultAllocatorCallback;
	physx::PxSimulationFilterShader defaultFilterShader;

	physx::PxFoundation *foundation;
	physx::PxPhysics *physics;
	physx::PxMaterial *defaultMaterial;
	physx::PxScene *scene;
	physx::PxGpuDispatcher *gpuDispatcher;
	physx::PxCudaContextManager *cudaContextManager;
	bool useGPUAcceleration;

#ifdef PVD_ENABLED
	physx::PxPvd *visualDebugger;
	physx::PxPvdTransport *pvdTransport;
	bool isPvdConnected;
#endif

	std::vector<NativeParticleSystem *> particleSystems;
	std::vector<NativeRigidBody *> rigidbodies;

	NativePhysicsEngine::NativePhysicsEngine(const PhysicsEngineConfiguration &config):
		PhysicsEngine(config),
		foundation(nullptr),
		physics(nullptr),
		defaultErrorCallback({}),
		defaultAllocatorCallback({}),
		defaultFilterShader(physx::PxSimulationFilterShader()),
		defaultMaterial(nullptr),
		scene(nullptr),
		gpuDispatcher(nullptr),
		cudaContextManager(nullptr),
		useGPUAcceleration(false) {
		Initialize(config);
	}

	NativePhysicsEngine::~NativePhysicsEngine() {
		Shutdown();
	}

	void ClearScene() {
		if(!isInitialized) return;

		for(size_t i = 0, count = particleSystems.size(); i < count; ++i) {
			NativeParticleSystem *particleSystem = particleSystems[i];
			if(particleSystem->isReady) {
				scene->removeActor(*particleSystem->fluid);
				particleSystem->isReady = false;
			}
			delete particleSystem;
		}
		particleSystems.clear();

		for(size_t i = 0, count = rigidbodies.size(); i < count; ++i) {
			NativeRigidBody *rigidbody = rigidbodies[i];
			if(rigidbody->isReady) {
				scene->removeActor(*rigidbody->actor);
				rigidbody->isReady = false;
			}
			delete rigidbody;
			// Do not delete, it will be deleted outside
		}
		rigidbodies.clear();

		// Clear the PhysicsEngine actors, because we already have removed it
		actors.clear();

		physx::PxActorTypeFlags flags =
			physx::PxActorTypeFlag::ePARTICLE_FLUID |
			physx::PxActorTypeFlag::ePARTICLE_SYSTEM |
			physx::PxActorTypeFlag::eCLOTH |
			physx::PxActorTypeFlag::eRIGID_DYNAMIC |
			physx::PxActorTypeFlag::eRIGID_STATIC;
		size_t actorCount = scene->getNbActors(flags);
		assert(actorCount == 0);
	}

	void Shutdown() {
		// Clear and release scene
		if(scene != nullptr) {
			ClearScene();
			scene->release();
			scene = nullptr;
		}

		// Release default material
		if(defaultMaterial != nullptr) {
			defaultMaterial->release();
			defaultMaterial = nullptr;
		}

		// Release GPU acceleration
		if(gpuDispatcher != nullptr) {
			gpuDispatcher = nullptr;
		}
		if(cudaContextManager) {
			cudaContextManager->release();
			cudaContextManager = nullptr;
		}

#ifdef PVD_ENABLED
		// Disconnect and release visual debugger
		if(visualDebugger != nullptr) {
			if(visualDebugger->isConnected())
				visualDebugger->disconnect();
			visualDebugger->release();
			visualDebugger = nullptr;
		}
		// TODO(final): Release transport?
		pvdTransport = nullptr;
		isPvdConnected = false;
#endif

		// Release physics and foundation
		if(physics != nullptr) {
			physics->release();
			physics = nullptr;
		}
		if(foundation != nullptr) {
			foundation->release();
			foundation = nullptr;
		}
	}

	void Initialize(const PhysicsEngineConfiguration &config) {
		isInitialized = false;

#ifdef PVD_ENABLED
		visualDebugger = nullptr;
		pvdTransport = nullptr;
		isPvdConnected = false;
#endif

		std::cout << "  PhysX Version: " << PX_PHYSICS_VERSION_MAJOR << "." << PX_PHYSICS_VERSION_MINOR << "." << PX_PHYSICS_VERSION_BUGFIX << std::endl;
		std::cout << "  PhysX Foundation Version: " << PX_FOUNDATION_VERSION_MAJOR << "." << PX_FOUNDATION_VERSION_MINOR << "." << PX_FOUNDATION_VERSION_BUGFIX << std::endl;

		// Create instance
		defaultErrorCallback = physx::PxDefaultErrorCallback();
		defaultAllocatorCallback = physx::PxDefaultAllocator();
		defaultFilterShader = physx::PxDefaultSimulationFilterShader;
		foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, defaultAllocatorCallback, defaultErrorCallback);
		if(foundation == nullptr) {
			std::cerr << "  Failed creating PhysX foundation instance!" << std::endl;
			Shutdown();
			return;
		}

		physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale());
		if(physics == nullptr) {
			std::cerr << "  Failed creating PhysX instance!" << std::endl;
			Shutdown();
			return;
		}

		physx::PxPvd *thisPvd = nullptr;

#ifdef PVD_ENABLED
		// Connect to visual debugger
		isPvdConnected = false;
		visualDebugger = PxCreatePvd(*foundation);
		if(visualDebugger != nullptr) {
			pvdTransport = physx::PxDefaultPvdSocketTransportCreate(PVD_Host, PVD_Port, 10000);
			if(pvdTransport != nullptr) {
				printf("  Connect to PVD on host '%s' with port %d\n", PVD_Host, PVD_Port);
				if(!visualDebugger->connect(*pvdTransport, physx::PxPvdInstrumentationFlag::eALL)) {
					printf("  Failed to connect to PVD on host '%s' with port %d!\n", PVD_Host, PVD_Port);
				} else {
					printf("  Successfully connected to PVD on host '%s' with port %d!\n", PVD_Host, PVD_Port);
					isPvdConnected = true;
				}
			} else {
				printf("  Failed creating transport for host '%s' with port %d!\n", PVD_Host, PVD_Port);
			}
		} else {
			std::cerr << "  Failed creating visual debugger for PhysX, skip it!" << std::endl;
		}
		if(!isPvdConnected) {
			// Error release pvd resources
			if(pvdTransport != nullptr) {
				pvdTransport->release();
				pvdTransport = nullptr;
			}
			if(visualDebugger != nullptr) {
				visualDebugger->release();
				visualDebugger = nullptr;
			}
		}

		thisPvd = visualDebugger;
#endif

		// Initialize PhysX Extensions
		if(!PxInitExtensions(*physics, thisPvd)) {
			std::cerr << "  Failed to initialize PhysX extensions!" << std::endl;
			Shutdown();
			return;
		}

		// Create the scene
		physx::PxSceneDesc sceneDesc(physics->getTolerancesScale());
		sceneDesc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);

		// Default filter shader (No idea whats that about)
		sceneDesc.filterShader = defaultFilterShader;

		// CPU Dispatcher based on number of cpu cores
		uint32_t coreCount = COSLowLevel::getNumCPUCores();
		uint32_t numThreads = std::min(config.threadCount, coreCount);
		printf("  CPU core count: %lu\n", coreCount);
		printf("  CPU acceleration supported (%d threads)\n", numThreads);
		sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(numThreads);

		// GPU Dispatcher
		useGPUAcceleration = false;
		physx::PxCudaContextManagerDesc cudaContextManagerDesc;
		cudaContextManager = PxCreateCudaContextManager(*foundation, cudaContextManagerDesc);
		if(cudaContextManager != nullptr) {
			gpuDispatcher = cudaContextManager->getGpuDispatcher();
			if(gpuDispatcher != nullptr) {
				const char *deviceName = cudaContextManager->getDeviceName();
				printf("  GPU acceleration supported, using device: %s\n", deviceName);
				useGPUAcceleration = true;
				sceneDesc.gpuDispatcher = gpuDispatcher;
			}
		}

		// Default material
		defaultMaterial = physics->createMaterial(0.3f, 0.3f, 0.1f);
		if(defaultMaterial == nullptr) {
			std::cerr << "Failed to create default material!" << std::endl;
			Shutdown();
			return;
		}

		// Create scene
		printf("  Creating scene\n");
		scene = physics->createScene(sceneDesc);
		if(scene == nullptr) {
			std::cerr << "Failed to create the scene!" << std::endl;
			Shutdown();
			return;
		}

		scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
		scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

		isInitialized = true;
	}

	void Advance(const float deltaTime) {
		scene->simulate(deltaTime);
		scene->fetchResults(true);
	}

	void Syncronize() {
		physx::PxShape *shapes[PhysicsRigidBody::MaxShapeCount];

		for(size_t bodyIndex = 0, count = rigidbodies.size(); bodyIndex < count; ++bodyIndex) {
			NativeRigidBody *rigidbody = rigidbodies[bodyIndex];

			physx::PxRigidActor *nrigidActor = rigidbody->actor;

			physx::PxTransform worldTransform = nrigidActor->getGlobalPose();
			rigidbody->transform.rotation = PhysicsUtils::toGLMQuat(worldTransform.q);
			rigidbody->transform.pos = PhysicsUtils::toGLMVec3(worldTransform.p);

			physx::PxBounds3 nbounds = nrigidActor->getWorldBounds();
			rigidbody->bounds = PhysicsBoundingBox(PhysicsUtils::toGLMVec3(nbounds.minimum), PhysicsUtils::toGLMVec3(nbounds.maximum));

			physx::PxU32 shapeCount = nrigidActor->getNbShapes();
			physx::PxU32 writtenShapeCount = nrigidActor->getShapes(shapes, shapeCount);
			assert(writtenShapeCount == shapeCount);

			for(physx::PxU32 shapeIndex = 0; shapeIndex < shapeCount; ++shapeIndex) {
				physx::PxShape *nshape = shapes[shapeIndex];
				physx::PxGeometryType::Enum geoType = nshape->getGeometryType();
				physx::PxTransform localTransform = nshape->getLocalPose();

				PhysicsShape &targetShape = rigidbody->shapes[shapeIndex];

				targetShape.local.rotation = PhysicsUtils::toGLMQuat(localTransform.q);
				targetShape.local.pos = PhysicsUtils::toGLMVec3(localTransform.p);

				switch(geoType) {
					case physx::PxGeometryType::ePLANE:
					{
						// No code needed because we already have the transform
					} break;

					case physx::PxGeometryType::eBOX:
					{
						physx::PxBoxGeometry nbox;
						if(nshape->getBoxGeometry(nbox)) {
							targetShape.box.halfExtents = PhysicsUtils::toGLMVec3(nbox.halfExtents);
						}
					} break;

					case physx::PxGeometryType::eSPHERE:
					{
						physx::PxSphereGeometry nsphere;
						if(nshape->getSphereGeometry(nsphere)) {
							targetShape.sphere.radius = nsphere.radius;
						}
					} break;

					case physx::PxGeometryType::eCAPSULE:
					{
						physx::PxCapsuleGeometry ncapsule;
						if(nshape->getCapsuleGeometry(ncapsule)) {
							targetShape.capsule.radius = ncapsule.radius;
							targetShape.capsule.halfHeight = ncapsule.halfHeight;
						}
					} break;
				}
			}
		}

		for(size_t i = 0, count = particleSystems.size(); i < count; ++i) {
			NativeParticleSystem *particleSys = particleSystems[i];
			particleSys->Syncronize();
		}
	}

	void Simulate(const float deltaTime) {
		Advance(deltaTime);
		Syncronize();
	}

	void Clear() {
		ClearScene();
		PhysicsEngine::Clear();
	}

	void AddActor(PhysicsActor *actor) {
		if(!isInitialized) return;

		actor->isReady = false;

		physx::PxActor *nactor = nullptr;
		if(actor->type == PhysicsActor::Type::RigidBody) {
			NativeRigidBody *nRigidBody = static_cast<NativeRigidBody *>(actor);
			rigidbodies.push_back(nRigidBody);
			nactor = nRigidBody->actor;
		} else if(actor->type == PhysicsActor::Type::ParticleSystem) {
			NativeParticleSystem *nParticleSystem = static_cast<NativeParticleSystem *>(actor);
			particleSystems.push_back(nParticleSystem);
			nactor = nParticleSystem->fluid;
		} else {
			assert(!"Not supported");
		}

		if(nactor != nullptr) {
			scene->addActor(*nactor);
			actor->isReady = true;
		}
	}

	void RemoveActor(PhysicsActor *actor) {
		if(!isInitialized) return;

		physx::PxActor *nactor = nullptr;
		if(actor->type == PhysicsActor::Type::RigidBody) {
			NativeRigidBody *nRigidBody = static_cast<NativeRigidBody *>(actor);
			nactor = nRigidBody->actor;
			rigidbodies.erase(std::remove(rigidbodies.begin(), rigidbodies.end(), nRigidBody), rigidbodies.end());
		} else if(actor->type == PhysicsActor::Type::ParticleSystem) {
			NativeParticleSystem *nParticleSystem = static_cast<NativeParticleSystem *>(actor);
			nactor = nParticleSystem->fluid;
			particleSystems.erase(std::remove(particleSystems.begin(), particleSystems.end(), nParticleSystem), particleSystems.end());
		} else {
			assert(!"Not supported");
		}

		if(nactor != nullptr && actor->isReady) {
			scene->removeActor(*nactor);
			actor->isReady = false;
		}
	}

	PhysicsParticleSystem *CreateParticleSystem(const FluidSimulationProperties &desc, const uint32_t maxParticleCount) {
		if(!isInitialized) return(nullptr);
		NativeParticleSystem *result = new NativeParticleSystem(physics, useGPUAcceleration, desc, maxParticleCount);
		return(result);
	}

	bool AddParticles(PhysicsParticleSystem *particleSystem, const PhysicsParticlesStorage &storage) {
		if(!isInitialized || particleSystem == nullptr) return(false);
		NativeParticleSystem *nativeParticleSys = static_cast<NativeParticleSystem *>(particleSystem);
		bool result = nativeParticleSys->AddParticles(storage);
		return(result);
	}

	PhysicsRigidBody *CreateRigidBody(const PhysicsRigidBody::MotionKind motionKind, const glm::vec3 &pos, const glm::quat &rotation, const PhysicsShape &shape) {
		PhysicsRigidBody *nativeRigidBody = new NativeRigidBody(physics, scene, defaultMaterial, motionKind, pos, rotation, shape);
		return(nativeRigidBody);
	}

	bool SupportsGPUAcceleration() {
		bool result = isInitialized && gpuDispatcher != nullptr;
		return(result);

	}

	bool IsGPUAcceleration() {
		bool result = isInitialized && gpuDispatcher != nullptr && useGPUAcceleration;
		return(result);
	}

	void SetGPUAcceleration(const bool value) {
		if(!isInitialized)return;
		if(gpuDispatcher != nullptr) {
			useGPUAcceleration = value;

			// Need to update the particle system flags, but before we need to remove it from the scene first
			for(size_t i = 0, count = particleSystems.size(); i < count; ++i) {
				NativeParticleSystem *particleSystem = particleSystems[i];
				if(particleSystem->isReady) {
					scene->removeActor(*particleSystem->fluid);
					particleSystem->fluid->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, value);
					scene->addActor(*particleSystem->fluid);
				}
			}
		}
	}

	void SetGravity(const glm::vec3 &gravity) {
		if(!isInitialized)return;
		scene->setGravity(PhysicsUtils::toPxVec3(gravity));
	}

};

PhysicsEngine::PhysicsEngine(const PhysicsEngineConfiguration &config):
	isInitialized(false),
	stepDT(config.deltaTime),
	accumulator(0) {
}

PhysicsEngine::~PhysicsEngine() {
	Clear();
}

PhysicsEngine *PhysicsEngine::Create(const PhysicsEngineConfiguration &config) {
	NativePhysicsEngine *result = new NativePhysicsEngine(config);
	return(result);
}

void PhysicsEngine::Clear() {
	for(size_t i = 0, count = actors.size(); i < count; ++i) {
		PhysicsActor *actor = actors[i];
		delete actor;
	}
	actors.clear();
	accumulator = 0;
}

void PhysicsEngine::Step(const float dt) {
	assert(stepDT > 0);
	accumulator += dt;
	while(accumulator >= stepDT) {
		Simulate(stepDT);
		accumulator -= stepDT;
	}
}

PhysicsParticleSystem *PhysicsEngine::AddParticleSystem(const FluidSimulationProperties &desc, const uint32_t maxParticleCount) {
	PhysicsParticleSystem *result = CreateParticleSystem(desc, maxParticleCount);
	if(result != nullptr) {
		AddActor(result);
		actors.push_back(result);
	}
	return(result);
}

void PhysicsEngine::DeleteParticleSystem(PhysicsParticleSystem *particleSystem) {
	if(particleSystem != nullptr) {
		RemoveActor(particleSystem);
		actors.erase(std::remove(actors.begin(), actors.end(), particleSystem), actors.end());
		delete particleSystem;
	}
}

PhysicsRigidBody *PhysicsEngine::AddRigidBody(const PhysicsRigidBody::MotionKind motionKind, const glm::vec3 &pos, const glm::quat &rotation, const PhysicsShape &shape) {
	PhysicsRigidBody *result = CreateRigidBody(motionKind, pos, rotation, shape);
	if(result != nullptr) {
		AddActor(result);
		actors.push_back(result);
	}
	return(result);
}

void PhysicsEngine::DeleteRigidBody(PhysicsRigidBody *body) {
	if(body != nullptr) {
		RemoveActor(body);
		actors.erase(std::remove(actors.begin(), actors.end(), body), actors.end());
		delete body;
	}
}