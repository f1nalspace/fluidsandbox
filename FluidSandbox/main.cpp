/*
======================================================================================================================
	Fluid Sandbox

	A realtime application for playing around with 3D fluids and rigid bodies.
	Features
	- Fluid and rigid body simulation based on NVidia PhysX
	- Screen Space Fluid Rendering with clear and colored rendering
	- Customizable Scene & Scenarios
	- Custom Rendering Engine

	This is the main source file.

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v.2 licensed. See LICENSE.txt for more details.
======================================================================================================================
Dependencies:
	- Windows 7 or higher
	- Visual Studio 2019

	- PhysX SDK 3.4.2 (Multithreaded DLL, x64/win32, Headers are included)

	- STB image (included)
	- STB freetype (included)
	- Final Platform Layer (included)
	- Glad (included)
	- glm (included)
	- rapidxml (included)
======================================================================================================================
How to compile:
	PhysX SDK:
		- Compile the PhysX SDK as Multithreaded DLLs for x64 and x64 platform with the configurations Debug/Release
			NOTE: Do not compile with static CRT library, change to (Multithreaded DLL and Multithreaded Debug DLL).

		- Copy all 10 resulting PhysX DLL files into the folders for each platform:
			- /Libraries/PhysX/physx/bin/win32/
				- nvToolsExt32_1.dll
				- PhysX3_x86.dll
				- PhysX3Common_x86.dll
				- PhysX3CommonDEBUG_x86.dll
				- PhysX3Cooking_x86.dll
				- PhysX3CookingDEBUG_x86.dll
				- PhysX3DEBUG_x86.dll
				- PhysX3Gpu_x86.dll
				- PhysX3GpuDEBUG_x86.dll
				- PhysXDevice.dll

			- /Libraries/PhysX/physx/bin/x64/
				- nvToolsExt32_1.dll
				- PhysX3_x64.dll
				- PhysX3Common_x64.dll
				- PhysX3CommonDEBUG_x64.dll
				- PhysX3Cooking_x64.dll
				- PhysX3CookingDEBUG_x64.dll
				- PhysX3DEBUG_x64.dll
				- PhysX3Gpu_x64.dll
				- PhysX3GpuDEBUG_x64.dll
				- PhysXDevice.dll

		- Copy all 8 resulting PhysX LIB files into the folders respectively:
			- /Libraries/PhysX/physx/lib/win32/
				- PhysX3_x86.lib
				- PhysX3Common_x86.lib
				- PhysX3CommonDEBUG_x86.lib
				- PhysX3Cooking_x86.lib
				- PhysX3CookingDEBUG_x86.lib
				- PhysX3DEBUG_x86.lib
				- PhysX3Extensions.lib
				- PhysX3ExtensionsDEBUG.lib

			- /Libraries/PhysX/physx/lib/x64/
				- PhysX3_x64.lib
				- PhysX3Common_x64.lib
				- PhysX3CommonDEBUG_x64.lib
				- PhysX3Cooking_x64.lib
				- PhysX3CookingDEBUG_x64.lib
				- PhysX3DEBUG_x64.lib
				- PhysX3Extensions.lib
				- PhysX3ExtensionsDEBUG.lib

		- Copy the include files for PhysX SDK in the folder:
			- /Libraries/PhysX/pxshared/include/ (~160 files)
			- /Libraries/PhysX/physx/include/ (~60 files)

	- All other libraries are already included

	- Build and run the FluidSandbox Solution
======================================================================================================================
Todo:

	- Replace Glad with final_dynamic_opengl.h
	- Replace rapidxml with final_xml.h

	- Move all physics code into its own class, so we can swap physics engine any time (Allmost done)

	- Use ImGUI for OSD, so we dont have to use keyboard to modify the scene

	- Migrate to OpenGL 3.x

	- Abstract rendering so we can support multiple renderer (GL 3.x, Vulkan)

	- More cameras (Free, Rotate around point, Fixed)

	- Transform the sandbox in a real sandbox, by making it a 3D editor (ImGUI)

	- Add actor animations (Useful for simulating waves):
		- Rotation
		- Simple movement

	- Support for kinematic bodies (Moveable static bodies)

	- Support for joints

	- Generate tubes (http://www.songho.ca/opengl/gl_cylinder.html)

	- More scenarios:
		- Use custom gravity
		- Tubes
		- Water slides
		- Rube Goldberg machine

======================================================================================================================
License:

	This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0
	You can find a a copy of the license file in the source directory (LICENSE.txt)
	If a copy of the MPL was not distributed with this file, You can obtain one at https://www.mozilla.org/MPL/2.0/.

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
======================================================================================================================
*/

// Enable this to activate support for PhysX Visual Debugger
//#define PVD_ENABLED

#define FPL_ENTRYPOINT
#define FPL_NO_AUDIO
#include <final_platform_layer.h>

#include <iostream>
#include <vector>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <math.h>
#include <typeinfo>

// OpenGL
#include <glad/glad.h>

// Vector math
#include <glm/glm.hpp>
#include <glm/gtc\matrix_transform.hpp>
#include <glm/gtc\quaternion.hpp>

// PhysX API
#include <PxPhysicsAPI.h>

// Classes
#include "Frustum.h"
#include "OSLowLevel.h"
#include "GLSL.h"
#include "SphericalPointSprites.h"
#include "Renderer.h"
#include "Camera.hpp"
#include "Utils.h"
#include "ScreenSpaceFluidRendering.h"
#include "Scenario.h"
#include "Actor.hpp"
#include "Scene.h"
#include "FluidSystem.h"
#include "Primitives.h"
#include "TextureManager.h"
#include "FluidProperties.h"
#include "GeometryVBO.h"

// Font
#include "TextureFont.h"
#include "Fonts.h"

#include "AllShaders.hpp"
#include "AllFBOs.hpp"
#include "AllActors.hpp"

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
	return glm::quat(input.x, input.y, input.z, input.w);
}

// Application
const char *APPLICATION_NAME = "Fluid Sandbox";

#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
#	ifdef _DEBUG
const char *APPLICATION_VERSION = "1.8.0 (x64-debug)";
#	else
const char *APPLICATION_VERSION = "1.8.0 (x64-release)";
#	endif
#else
#	ifdef _DEBUG
const char *APPLICATION_VERSION = "1.8.0 (x86-debug)";
#	else
const char *APPLICATION_VERSION = "1.8.0 (x86-release)";
#	endif
#endif

const char *APPLICATION_AUTHOR = "Torsten Spaete";
const char *APPLICATION_COPYRIGHT = "(C) 2015-2021 Torsten Spaete - All rights reserved";
const std::string APPTITLE = APPLICATION_NAME + std::string(" v") + APPLICATION_VERSION + std::string(" by ") + APPLICATION_AUTHOR;

// Math stuff
const float DEG2RAD = (float)M_PI / 180.0f;

//
// PhysX
//
//

constexpr float PhysXSimulationDT = 1.0f / 60.0f;
static physx::PxFoundation *gPhysXFoundation = nullptr;
static physx::PxPhysics *gPhysicsSDK = nullptr;
static physx::PxDefaultErrorCallback gDefaultErrorCallback;
static physx::PxDefaultAllocator gDefaultAllocatorCallback;
static physx::PxSimulationFilterShader gDefaultFilterShader = physx::PxDefaultSimulationFilterShader;
static physx::PxMaterial *gDefaultMaterial = nullptr;
static physx::PxScene *gScene = nullptr;

#ifdef PVD_ENABLED
const char *PVD_Host = "localhost";
const int PVD_Port = 5425;
static physx::PxPvd *gPhysXVisualDebugger = nullptr;
static physx::PxPvdTransport *gPhysXPvdTransport = nullptr;
static bool gIsPhysXPvdConnected = false;
#endif

static physx::PxGpuDispatcher *gGPUDispatcher = nullptr;
static physx::PxCudaContextManager *gCudaContextManager = nullptr;

// Window vars
constexpr int DefaultWindowWidth = 1280;
constexpr int DefaultWindowHeight = 720;
constexpr float DefaultFov = 60.0;
constexpr float DefaultZNear = 0.1f;
constexpr float DefaultZFar = 1000.0f;

// Rigid bodies settings

enum class ActorCreationKind: int {
	RigidBox = 0,
	RigidSphere,
	RigidCapsule,
	FluidDrop,
	FluidPlane,
	FluidCube,
	FluidSphere,
	Max = FluidSphere
};
static ActorCreationKind gCurrentActorCreationKind = ActorCreationKind::FluidCube;

static std::vector<Actor *> gActors;

constexpr int HideRigidBody_None = 0;
constexpr int HideRigidBody_Blending = 1;
constexpr int HideRigidBody_All = 2;
constexpr int HideRigidBody_MAX = HideRigidBody_All;

// Render states
static bool gDrawWireframe = false;
static bool gDrawBoundBox = false;
static int gHideRigidBodies = HideRigidBody_None;
static bool gShowOSD = false;

// Drawing statistics
static size_t gTotalActors = 0;
static size_t gDrawedActors = 0;
static uint32_t gTotalFluidParticles = 0;
static float gFps = 0;
static int gTotalFrames = 0;
static float gAppStartTime = 0.0f;

// For simulation
constexpr float DefaultRigidBodyDensity = 0.05f;
constexpr static glm::vec3 DefaultRigidBodyVelocity(0.0f, 0.0f, 0.0f);
constexpr float InitPhysicsDT = 0.000001f;

// Fluid
constexpr int MaxFluidParticleCount = 512000;

static CFluidSystem *gFluidSystem = nullptr;
static CSphericalPointSprites *gPointSprites = nullptr;
static CPointSpritesShader *gPointSpritesShader = nullptr;

static bool gFluidUseGPUAcceleration = false;
static FluidDebugType gFluidDebugType = FluidDebugType::Final;

// Fluid properties
// TODO(final): Use FluidProperties instead here!
static float gFluidViscosity = 0.0f;
static float gFluidStiffness = 0.0f;
static float gFluidRestOffset = 0.0f;
static float gFluidContactOffset = 0.0f;
static float gFluidRestParticleDistance = 0.0f;
static float gFluidMaxMotionDistance = 0.0f;
static float gFluidRestitution = 0.0f;
static float gFluidDamping = 0.0f;
static float gFluidDynamicFriction = 0.0f;
static float gFluidStaticFriction = 0.0f;
static float gFluidParticleMass = 0.0f;
static float gFluidParticleRadius = 0.0f;
static float gFluidParticleRenderFactor = 0.0f;
static float gFluidCellSize = 0.0f;

// Fluid modification
static int64_t gFluidLatestExternalAccelerationTime = -1;

// Fluid property realtime change
enum class FluidProperty : int {
	None = 0,
	Viscosity,
	Stiffness,
	MaxMotionDistance,
	ContactOffset,
	RestOffset,
	Restitution,
	Damping,
	DynamicFriction,
	StaticFriction,
	ParticleMass,
	DepthBlurScale,
	ParticleRenderFactor,
	ColorFalloffScale,
	ColorFalloffAlpha,
	DebugType,
	First = Viscosity,
	Last = DebugType
};

static FluidProperty gFluidCurrentProperty = FluidProperty::None;

// Scenario
static bool gStoppedEmitter = false;
static glm::vec3 gRigidBodyFallPos(0.0f, 10.0f, 0.0f);
static std::vector<Scenario *> gFluidScenarios;
static Scenario *gActiveFluidScenario = nullptr;
static int gActiveFluidScenarioIdx = -1;
static bool gWaterAddBySceneChange = true;

// Renderer
static CRenderer *gRenderer = nullptr;

// Fluid Renderer
static CScreenSpaceFluidRendering *gFluidRenderer = nullptr;
static SSFRenderMode gSSFRenderMode = SSFRenderMode::Fluid;
static float gSSFDetailFactor = 1.0f;
static float gSSFBlurDepthScale = 0.0008f;
static bool gSSFBlurActive = true;
static int gSSFCurrentFluidIndex = 0; // // Current fluid color index

// Managers
static CTextureManager *gTexMng = nullptr;

// Scene
static CScene *gActiveScene = nullptr;

// Current camera
static glm::vec2 gCamRotation = glm::vec2(15, 0);
static float gCameraDistance = 15;
static CCamera gCamera;
static Frustum gFrustum;

// Non fluid rendering
static CLightingShader *gLightingShader = nullptr;

static CSceneFBO *gSceneFBO = nullptr;
static CGLSL *gSceneShader = nullptr;
static GeometryVBO *gSkyboxVBO = nullptr;
static CSkyboxShader *gSkyboxShader = nullptr;
static CTexture *gSkyboxCubemap = nullptr;

static GeometryVBO *gBoxVBO = nullptr;
static GeometryVBO *gSphereVBO = nullptr;
static GeometryVBO *gCylinderVBO = nullptr;

static FontAtlas *gFontAtlas16 = nullptr;
static FontAtlas *gFontAtlas32 = nullptr;
static CTextureFont *gFontTexture16 = nullptr;
static CTextureFont *gFontTexture32 = nullptr;

// Timing
static float gTotalTimeElapsed = 0;
static float gPhysicsAccumulator = 0;
static bool gPaused = false;

// Default colors
constexpr static glm::vec4 DefaultStaticRigidBodyColor(0.0f, 0.0f, 0.1f, 0.3f);
constexpr static glm::vec4 DefaultDynamicRigidBodyCubeColor(0.85f, 0.0f, 0.0f, 1.0f);
constexpr static glm::vec4 DefaultDynamicRigidBodySphereColor(0, 0.85f, 0.0f, 1.0f);
constexpr static glm::vec4 DefaultDynamicRigidBodyCapsuleColor(0.85f, 0.85f, 0.0f, 1.0f);

// Deegree to radius
float Deg2Rad(float degree) {
	return degree * DEG2RAD;
}

// Returns a random radius
float RandomRadius() {
	return (float)(rand() % 360);
}

// Returns a random float in range -1 to 1
float getRandomFloat(float min, float max) {
	float scale = RAND_MAX + 1.;
	float base = rand() / scale;
	float fine = rand() / scale;
	return min + ((base + fine / scale) * (max - min));
}

void setActorDrain(physx::PxActor *actor, const bool enable) {
	assert(actor != nullptr);

	physx::PxType actorType = actor->getConcreteType();
	if(actorType == physx::PxConcreteType::eRIGID_STATIC || actorType == physx::PxConcreteType::eRIGID_DYNAMIC) {
		physx::PxRigidActor *rigActor = (physx::PxRigidActor *)actor;
		physx::PxU32 shapeCount = rigActor->getNbShapes();
		physx::PxShape **shapes = new physx::PxShape * [shapeCount];
		rigActor->getShapes(shapes, shapeCount);

		for(physx::PxU32 shapeIndex = 0; shapeIndex < shapeCount; ++shapeIndex) {
			physx::PxShapeFlags flags = shapes[shapeIndex]->getFlags();
			if(enable)
				flags |= physx::PxShapeFlag::ePARTICLE_DRAIN;
			else
				flags &= ~physx::PxShapeFlag::ePARTICLE_DRAIN;
			shapes[shapeIndex]->setFlags(flags);
		}

		delete[] shapes;
	}
}

std::string vecToString(const physx::PxVec3 &p) {
	char str[255];
	sprintf_s(str, "%f, %f, %f", p.x, p.y, p.z);
	return str;
}

struct PhysXActorProperties {
	physx::PxVec3 vel;
	physx::PxTransform transform;

	PhysXActorProperties(const Actor &actor) {
		vel = toPxVec3(actor.velocity);
		physx::PxVec3 pos = toPxVec3(actor.transform.position);
		physx::PxQuat rot = toPxQuat(actor.transform.rotation);
		transform = physx::PxTransform(pos, rot);
	}
};

static void AddBox(physx::PxScene &scene, CubeActor *cube) {
	PhysXActorProperties props = PhysXActorProperties(*cube);
	physx::PxReal density = cube->density;
	physx::PxVec3 halfExtents = toPxVec3(cube->halfExtents);
	physx::PxBoxGeometry geometry(halfExtents);
	physx::PxActor *actor;
	if(cube->movementType == ActorMovementType::Static) {
		actor = PxCreateStatic(*gPhysicsSDK, props.transform, geometry, *gDefaultMaterial);
		assert(actor != nullptr);
	} else {
		physx::PxRigidDynamic *rigidbody = PxCreateDynamic(*gPhysicsSDK, props.transform, geometry, *gDefaultMaterial, density);
		assert(rigidbody != nullptr);
		actor = rigidbody;
		physx::PxRigidBodyExt::updateMassAndInertia(*rigidbody, density);
		rigidbody->setAngularDamping(0.75);
		rigidbody->setLinearVelocity(props.vel);
	}

	setActorDrain(actor, cube->particleDrain);

	actor->userData = cube;
	cube->physicsData = actor;

	scene.addActor(*actor);
}

static void AddSphere(physx::PxScene &scene, SphereActor *sphere) {
	PhysXActorProperties props = PhysXActorProperties(*sphere);
	physx::PxReal density = sphere->density;
	physx::PxSphereGeometry geometry(sphere->radius);
	physx::PxActor *actor;
	if(sphere->movementType == ActorMovementType::Static) {
		actor = PxCreateStatic(*gPhysicsSDK, props.transform, geometry, *gDefaultMaterial);
		assert(actor != nullptr);
	} else {
		physx::PxRigidDynamic *rigidbody = PxCreateDynamic(*gPhysicsSDK, props.transform, geometry, *gDefaultMaterial, density);
		assert(rigidbody != nullptr);
		actor = rigidbody;
		physx::PxRigidBodyExt::updateMassAndInertia(*rigidbody, density);
		rigidbody->setAngularDamping(0.75);
		rigidbody->setLinearVelocity(props.vel);
	}

	setActorDrain(actor, sphere->particleDrain);

	actor->userData = sphere;
	sphere->physicsData = actor;

	scene.addActor(*actor);
}

static void AddCapsule(physx::PxScene &scene, CapsuleActor *capsule) {
	PhysXActorProperties props = PhysXActorProperties(*capsule);
	physx::PxReal density = capsule->density;
	physx::PxCapsuleGeometry geometry(capsule->radius, capsule->halfHeight);
	physx::PxActor *actor;
	if(capsule->movementType == ActorMovementType::Static) {
		actor = PxCreateStatic(*gPhysicsSDK, props.transform, geometry, *gDefaultMaterial);
		assert(actor != nullptr);
	} else {
		physx::PxRigidDynamic *rigidbody = PxCreateDynamic(*gPhysicsSDK, props.transform, geometry, *gDefaultMaterial, density);
		assert(rigidbody != nullptr);
		actor = rigidbody;
		physx::PxRigidBodyExt::updateMassAndInertia(*rigidbody, density);
		rigidbody->setAngularDamping(0.75);
		rigidbody->setLinearVelocity(props.vel);
	}

	setActorDrain(actor, capsule->particleDrain);

	actor->userData = capsule;
	capsule->physicsData = actor;

	scene.addActor(*actor);
}

inline bool PointInSphere(const physx::PxVec3 &spherePos, const float &sphereRadius, const physx::PxVec3 point, const float particleRadius) {
	physx::PxVec3 distance = spherePos - point;
	float length = distance.magnitude();
	float sumRadius = sphereRadius + particleRadius;
	return length <= sumRadius;
}

static void AddFluid(CFluidSystem &fluidSys, const FluidActor &container, const FluidType type) {
	uint32_t numParticles = 0;

	float distance = gFluidRestParticleDistance;

	physx::PxVec3 vel = toPxVec3(container.velocity);

	glm::vec3 center = container.transform.position;

	float centerX = center.x;
	float centerY = center.y;
	float centerZ = center.z;

	float sizeX = container.size.x;
	float sizeY = container.size.y;
	float sizeZ = container.size.z;

	float radius = container.radius;
	if(radius < 0.00001f) {
		radius = ((sizeX + sizeY + sizeZ) / 3.0f) / 2.0f;
	}

	long numX = (long)(sizeX / distance);
	long numY = (long)(sizeY / distance);
	long numZ = (long)(sizeZ / distance);

	float dX = distance * numX;
	float dY = distance * numY;
	float dZ = distance * numZ;

	int idx;

	std::vector<physx::PxVec3> particlePositionBuffer;
	std::vector<physx::PxVec3> particleVelocityBuffer;
	if(type == FluidType::Drop) {
		// Single drop
		numParticles++;
		particlePositionBuffer.push_back(physx::PxVec3(centerX, centerY, centerZ));
		particleVelocityBuffer.push_back(vel);
	} else if(type == FluidType::Plane) {
		// Water plane
		float zpos = centerZ - (dZ / 2.0f);
		idx = 0;

		for(long z = 0; z < numZ; z++) {
			float xpos = centerX - (dX / 2.0f);

			for(long x = 0; x < numX; x++) {
				numParticles++;
				particlePositionBuffer.push_back(physx::PxVec3(xpos, centerY, zpos));
				particleVelocityBuffer.push_back(vel);
				idx++;
				xpos += distance;
			}

			zpos += distance;
		}
	} else if(type == FluidType::Box) {
		// Water box
		float zpos = centerZ - (dZ / 2.0f);
		idx = 0;

		for(long z = 0; z < numZ; z++) {
			float ypos = centerY - (dY / 2.0f);

			for(long y = 0; y < numY; y++) {
				float xpos = centerX - (dX / 2.0f);

				for(long x = 0; x < numX; x++) {
					numParticles++;
					particlePositionBuffer.push_back(physx::PxVec3(xpos, ypos, zpos));
					particleVelocityBuffer.push_back(vel);
					idx++;
					xpos += distance;
				}

				ypos += distance;
			}

			zpos += distance;
		}
	} else if(type == FluidType::Sphere) {
		// Water sphere
		physx::PxVec3 center = physx::PxVec3(centerX, centerY, centerZ);

		float zpos = centerZ - (dZ / 2.0f);
		idx = 0;

		for(long z = 0; z < numZ; z++) {
			float ypos = centerY - (dY / 2.0f);

			for(long y = 0; y < numY; y++) {
				float xpos = centerX - (dX / 2.0f);

				for(long x = 0; x < numX; x++) {
					physx::PxVec3 point = physx::PxVec3(xpos, ypos, zpos);

					if(PointInSphere(center, radius, point, gFluidParticleRadius)) {
						numParticles++;
						particlePositionBuffer.push_back(point);
						particleVelocityBuffer.push_back(vel);
						idx++;
					}

					xpos += distance;
				}

				ypos += distance;
			}

			zpos += distance;
		}
	}

	if(numParticles > 0) {
		fluidSys.createParticles(numParticles, &particlePositionBuffer[0], &particleVelocityBuffer[0]);
	}
}

static void AddFluids(CFluidSystem &fluidSys, const FluidType type) {
	for(size_t i = 0, count = gActors.size(); i < count; i++) {
		Actor *actor = gActors[i];
		if(actor->type == ActorType::Fluid) {
			FluidActor *fluidActor = static_cast<FluidActor *>(actor);
			if(fluidActor->time <= 0) {
				AddFluid(fluidSys, *fluidActor, type);
			}
		}
	}
}

static void AddPlane(physx::PxScene &scene, PlaneActor *plane) {
	// TODO(final): Use plane position and rotation!

	// Create ground plane
	physx::PxTransform transform = physx::PxTransform(physx::PxVec3(0.0f, 0, 0.0f), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));

	physx::PxRigidStatic *actor = gPhysicsSDK->createRigidStatic(transform);

	physx::PxShape *shape = actor->createShape(physx::PxPlaneGeometry(), *gDefaultMaterial);

	actor->userData = plane;
	plane->physicsData = actor;

	scene.addActor(*actor);
}

static CFluidSystem *CreateParticleFluidSystem() {
	FluidSimulationProperties particleSystemDesc = FluidSimulationProperties();

	particleSystemDesc.stiffness = gFluidStiffness;
	particleSystemDesc.viscosity = gFluidViscosity;

	particleSystemDesc.restitution = gFluidRestitution;
	particleSystemDesc.damping = gFluidDamping;
	particleSystemDesc.dynamicFriction = gFluidDynamicFriction;
	particleSystemDesc.staticFriction = gFluidStaticFriction;
	particleSystemDesc.particleMass = gFluidParticleMass;
	particleSystemDesc.particleRadius = gFluidParticleRadius,

	particleSystemDesc.maxMotionDistance = gFluidMaxMotionDistance;
	particleSystemDesc.restParticleDistance = gFluidRestParticleDistance;
	particleSystemDesc.restOffset = gFluidRestOffset;
	particleSystemDesc.contactOffset = gFluidContactOffset;
	particleSystemDesc.cellSize = gFluidCellSize;

	particleSystemDesc.Validate();

	return new CFluidSystem(gPhysicsSDK, particleSystemDesc, MaxFluidParticleCount);
}

static void AddScenarioActor(physx::PxScene &scene, Actor *actor) {
	if(actor->type == ActorType::Cube) {
		CubeActor *cube = static_cast<CubeActor *>(actor);
		AddBox(scene, cube);
	} else if(actor->type == ActorType::Sphere) {
		SphereActor *sphere = static_cast<SphereActor *>(actor);
		AddSphere(scene, sphere);
	} else if(actor->type == ActorType::Capsule) {
		CapsuleActor *capsule = static_cast<CapsuleActor *>(actor);
		AddCapsule(scene, capsule);
	} else if(actor->type == ActorType::Plane) {
		PlaneActor *plane = static_cast<PlaneActor *>(actor);
		AddPlane(scene, plane);
	} else {
		assert(!"Actor type not supported");
	}
}

static void SaveFluidPositions() {
	float minDensity = gActiveFluidScenario != nullptr ? gActiveFluidScenario->render.minDensity : gActiveScene->render.minDensity;
	float *data = gPointSprites->Map();
	bool noDensity = gSSFRenderMode == SSFRenderMode::Points;
	gFluidSystem->writeToVBO(data, gTotalFluidParticles, noDensity, minDensity);
	gPointSprites->UnMap();
}

static void AdvanceSimulation(float &accumulator, float dt) {
	const physx::PxReal timestep = 1.0f / 60.0f;
	accumulator += dt;
	while(accumulator > 0.0f) {
		gScene->simulate(timestep);
		gScene->fetchResults(true);
		accumulator -= timestep;
	}
}

static void SingleStepPhysX(float &accumulator, const float frametime) {
	// Advance simulation
	AdvanceSimulation(accumulator, frametime);

	// Save fluid positions
	if(gSSFRenderMode != SSFRenderMode::Disabled)
		SaveFluidPositions();
}

static void ClearScene(physx::PxScene &scene) {
	// Remove fluid system
	if(gFluidSystem != nullptr) {
		scene.removeActor(*gFluidSystem->getActor());
		delete gFluidSystem;
		gFluidSystem = nullptr;
	}

	// Remove all actors
	for(size_t index = 0, count = gActors.size(); index < count; ++index) {
		Actor *actor = gActors[index];
		physx::PxActor *nactor = static_cast<physx::PxActor *>(actor->physicsData);
		if(nactor != nullptr) {
			scene.removeActor(*nactor);
			nactor->release();
		}
		delete actor;
	}
	gActors.clear();
}

static Actor *CloneBodyActor(const Actor *actor) {
	assert(actor != nullptr);
	switch(actor->type) {
		case ActorType::Plane:
		{
			const PlaneActor *sourceActor = static_cast<const PlaneActor *>(actor);
			PlaneActor *typedActor = new PlaneActor();
			typedActor->Assign(sourceActor);
			return(typedActor);
		}

		case ActorType::Cube:
		{
			const CubeActor *sourceActor = static_cast<const CubeActor *>(actor);
			CubeActor *typedActor = new CubeActor(sourceActor->movementType, sourceActor->halfExtents);
			typedActor->Assign(sourceActor);
			return(typedActor);
		}

		case ActorType::Sphere:
		{
			const SphereActor *sourceActor = static_cast<const SphereActor *>(actor);
			SphereActor *typedActor = new SphereActor(sourceActor->movementType, sourceActor->radius);
			typedActor->Assign(sourceActor);
			return(typedActor);
		}

		case ActorType::Capsule:
		{
			const CapsuleActor *sourceActor = static_cast<const CapsuleActor *>(actor);
			CapsuleActor *typedActor = new CapsuleActor(sourceActor->movementType, sourceActor->radius, sourceActor->halfHeight);
			typedActor->Assign(sourceActor);
			return(typedActor);
		}
	}
	return(nullptr);
}

static void ResetScene(physx::PxScene &scene) {
	assert(gActiveFluidScenario != nullptr);

	printf("Load/Reload scene: %s\n", gActiveFluidScenario->displayName);

	ClearScene(scene);

	// Set scene properties
	scene.setGravity(toPxVec3(gActiveFluidScenario->gravity));

	// Set particle properties
	gFluidMaxMotionDistance = gActiveFluidScenario->sim.maxMotionDistance;
	gFluidContactOffset = gActiveFluidScenario->sim.contactOffset;
	gFluidRestOffset = gActiveFluidScenario->sim.restOffset;
	gFluidRestitution = gActiveFluidScenario->sim.restitution;
	gFluidDamping = gActiveFluidScenario->sim.damping;
	gFluidDynamicFriction = gActiveFluidScenario->sim.dynamicFriction;
	gFluidParticleMass = gActiveFluidScenario->sim.particleMass;

	gFluidParticleRadius = gActiveFluidScenario->sim.particleRadius;
	gFluidCellSize = gActiveFluidScenario->sim.cellSize;
	gFluidViscosity = gActiveFluidScenario->sim.viscosity;
	gFluidStiffness = gActiveFluidScenario->sim.stiffness;
	gFluidRestParticleDistance = gActiveFluidScenario->sim.restParticleDistance;
	gRigidBodyFallPos = gActiveFluidScenario->actorCreatePosition;
	gFluidParticleRenderFactor = gActiveFluidScenario->render.particleRenderFactor;

	// Add ground plane
	PlaneActor *groundPlane = new PlaneActor();
	AddPlane(scene, groundPlane);
	gActors.push_back(groundPlane);

	// Create fluid system
	gFluidSystem = CreateParticleFluidSystem();
	assert(gFluidSystem != nullptr);

	// Set GPU acceleration for particle fluid if supported
	gFluidSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eCOLLISION_TWOWAY, true);
	gFluidSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, gGPUDispatcher && gFluidUseGPUAcceleration);

	// Add fluid system as actor
	physx::PxActor *nFluidActor = gFluidSystem->getActor();
	scene.addActor(*nFluidActor);

	// Add bodies immediately from scenario
	for(size_t i = 0, count = gActiveFluidScenario->bodies.size(); i < count; i++) {
		const Actor *sourceActor = gActiveFluidScenario->bodies[i];
		Actor *targetActor = CloneBodyActor(sourceActor);
		gActors.push_back(targetActor);
		if(targetActor != nullptr) {
			targetActor->timeElapsed = 0.0f;
			if(targetActor->time == -1) {
				AddScenarioActor(scene, targetActor);
			}
		}
	}

	// Add waters immediately from scenario
	for(size_t i = 0, count = gActiveFluidScenario->fluids.size(); i < count; i++) {
		const FluidActor *sourceActor = gActiveFluidScenario->fluids[i];
		FluidActor *targetActor = new FluidActor(sourceActor->size, sourceActor->radius, sourceActor->fluidType);
		targetActor->Assign(sourceActor);
		targetActor->timeElapsed = 0.0f;
		targetActor->emitterElapsed = 0.0f;
		targetActor->emitterCoolDownElapsed = 0.0f;
		targetActor->emitterCoolDownActive = false;
		gActors.push_back(targetActor);
		if(targetActor->time == -1 && !targetActor->isEmitter && gWaterAddBySceneChange) {
			AddFluid(*gFluidSystem, *targetActor, targetActor->fluidType);
		}
	}

	gTotalTimeElapsed = 0;
	gPhysicsAccumulator = 0;

	// Simulate physx one time
	SingleStepPhysX(gPhysicsAccumulator, InitPhysicsDT);
}

void InitializePhysX() {
	std::cout << "  PhysX Version: " << PX_PHYSICS_VERSION_MAJOR << "." << PX_PHYSICS_VERSION_MINOR << "." << PX_PHYSICS_VERSION_BUGFIX << std::endl;
	std::cout << "  PhysX Foundation Version: " << PX_FOUNDATION_VERSION_MAJOR << "." << PX_FOUNDATION_VERSION_MINOR << "." << PX_FOUNDATION_VERSION_BUGFIX << std::endl;

	// Create Physics SDK
	gPhysXFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gPhysXFoundation, physx::PxTolerancesScale());

	if(gPhysicsSDK == nullptr) {
		std::cerr << "  Could not create PhysX SDK, exiting!" << std::endl;
		exit(1);
	}

	// Initialize PhysX Extensions
	if(!PxInitExtensions(*gPhysicsSDK, nullptr)) {
		std::cerr << "  Could not initialize PhysX extensions, exiting!" << std::endl;
		exit(1);
	}

	// Connect to visual debugg1er
#ifdef PVD_ENABLED
	gIsPhysXPvdConnected = false;
	gPhysXVisualDebugger = PxCreatePvd(*gPhysXFoundation);
	if(gPhysXVisualDebugger != nullptr) {
		gPhysXPvdTransport = PxDefaultPvdSocketTransportCreate(PVD_Host, PVD_Port, 10000);
		if(gPhysXPvdTransport != nullptr) {
			printf("  Connect to PVD on host '%s' with port %d\n", PVD_Host, PVD_Port);
			if(!gPhysXVisualDebugger->connect(*gPhysXPvdTransport, PxPvdInstrumentationFlag::eALL)) {
				printf("  Failed to connect to PVD on host '%s' with port %d!\n", PVD_Host, PVD_Port);
			} else {
				printf("  Successfully connected to PVD on host '%s' with port %d!\n", PVD_Host, PVD_Port);
				gIsPhysXPvdConnected = true;
			}
		} else {
			printf("  Failed creating transport for host '%s' with port %d!\n", PVD_Host, PVD_Port);
		}
	} else {
		cerr << "  Failed creating visual debugger for PhysX, skip it!" << endl;
	}
#endif

	// Create the scene
	physx::PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);

	// Default filter shader (No idea whats that about)
	sceneDesc.filterShader = gDefaultFilterShader;

	// CPU Dispatcher based on number of cpu cores
	uint32_t coreCount = COSLowLevel::getNumCPUCores();
	uint32_t numThreads = std::min(gActiveScene->numCPUThreads, coreCount);
	printf("  CPU core count: %lu\n", coreCount);
	printf("  CPU acceleration supported (%d threads)\n", numThreads);
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(numThreads);

	// GPU Dispatcher
	gFluidUseGPUAcceleration = false;
	physx::PxCudaContextManagerDesc cudaContextManagerDesc;
	gCudaContextManager = PxCreateCudaContextManager(*gPhysXFoundation, cudaContextManagerDesc);

	if(gCudaContextManager) {
		gGPUDispatcher = gCudaContextManager->getGpuDispatcher();

		if(gGPUDispatcher) {
			printf("  GPU acceleration supported\n");
			gFluidUseGPUAcceleration = true;
			sceneDesc.gpuDispatcher = gGPUDispatcher;
		}
	}


	// Create scene
	printf("  Creating scene\n");
	gScene = gPhysicsSDK->createScene(sceneDesc);

	if(!gScene) {
		std::cerr << "Could not create scene, exiting!" << std::endl;
		exit(1);
	}

	gScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
	gScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

	gDefaultMaterial = gPhysicsSDK->createMaterial(0.3f, 0.3f, 0.1f);
}

glm::mat4 getColumnMajor(physx::PxMat33 m, physx::PxVec3 t) {
	glm::mat4 mat = glm::mat4(1.0f);

	mat[0][0] = m.column0[0];
	mat[0][1] = m.column0[1];
	mat[0][2] = m.column0[2];
	mat[0][3] = 0;

	mat[1][0] = m.column1[0];
	mat[1][1] = m.column1[1];
	mat[1][2] = m.column1[2];
	mat[1][3] = 0;

	mat[2][0] = m.column2[0];
	mat[2][1] = m.column2[1];
	mat[2][2] = m.column2[2];
	mat[2][3] = 0;

	mat[3][0] = t[0];
	mat[3][1] = t[1];
	mat[3][2] = t[2];
	mat[3][3] = 1;

	return mat;
}

void DrawGrid(int GRID_SIZE) {
	glBegin(GL_LINES);
	glColor3f(0.25f, 0.25f, 0.25f);

	for(int i = -GRID_SIZE; i <= GRID_SIZE; i++) {
		glVertex3f((float)i, 0, (float)-GRID_SIZE);
		glVertex3f((float)i, 0, (float)GRID_SIZE);

		glVertex3f((float)-GRID_SIZE, 0, (float)i);
		glVertex3f((float)GRID_SIZE, 0, (float)i);
	}

	glEnd();
}


void UpdatePhysX(const float frametime) {
	// Update water external direction if required
	if(gFluidLatestExternalAccelerationTime > -1) {
		uint64_t current = fplGetTimeInMillisecondsLP();
		if((int64_t)current > gFluidLatestExternalAccelerationTime) {
			gFluidSystem->setExternalAcceleration(physx::PxVec3(0.0f, 0.0f, 0.0f));
			gFluidLatestExternalAccelerationTime = -1;
		}
	}

	// Update PhysX
	if(!gPaused) {
		SingleStepPhysX(gPhysicsAccumulator, frametime);
	}
}

glm::vec4 getColor(physx::PxActor *actor, const glm::vec4 &defaultColor) {
	if(actor->userData) {
		Actor *a = (Actor *)actor->userData;
		return a->color;
	} else {
		physx::PxType actorType = actor->getConcreteType();
		if(actorType == physx::PxConcreteType::eRIGID_STATIC)
			return DefaultStaticRigidBodyColor;
		else
			return defaultColor;
	}
}

void DrawPrimitive(GeometryVBO *vbo, const bool asLines) {
	// Ensure that the matrix has correct transform for scale, position, rotation

	// Vertex (vec3, vec3, vec2)
	vbo->bind();

	if(!asLines || vbo->lineIndexCount == 0) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, pos)));
		glNormalPointer(GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, normal)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, texcoord)));
		gRenderer->DrawVBO(vbo, GL_TRIANGLES, vbo->triangleIndexCount, 0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	} else {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(Primitives::Vertex), (void *)(offsetof(Primitives::Vertex, pos)));
		gRenderer->DrawVBO(vbo, GL_LINES, vbo->lineIndexCount, sizeof(GLuint) * vbo->triangleIndexCount);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	vbo->unbind();
}

void DrawBoxShape(physx::PxShape *pShape) {
	physx::PxRigidActor *actor = pShape->getActor();
	glm::vec4 color = getColor(actor, DefaultDynamicRigidBodyCubeColor);

	physx::PxTransform pT = physx::PxShapeExt::getGlobalPose(*pShape, *actor);
	physx::PxBoxGeometry bg;
	pShape->getBoxGeometry(bg);
	physx::PxMat33 m = physx::PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 scaled = glm::scale(mat, glm::vec3(bg.halfExtents.x, bg.halfExtents.y, bg.halfExtents.z));
	glm::mat4 mvp = gCamera.mvp * scaled;
	gRenderer->LoadMatrix(mvp);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);
	DrawPrimitive(gBoxVBO, false);
	gLightingShader->disable();
}

void DrawSphereShape(physx::PxShape *pShape) {
	physx::PxRigidActor *actor = pShape->getActor();
	glm::vec4 color = getColor(actor, DefaultDynamicRigidBodySphereColor);

	physx::PxTransform pT = physx::PxShapeExt::getGlobalPose(*pShape, *actor);
	physx::PxSphereGeometry sg;
	pShape->getSphereGeometry(sg);
	physx::PxMat33 m = physx::PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 scaled = glm::scale(mat, glm::vec3(sg.radius));
	glm::mat4 mvp = gCamera.mvp * scaled;
	gRenderer->LoadMatrix(mvp);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);
	DrawPrimitive(gSphereVBO, false);
	gLightingShader->disable();
}

void DrawCapsuleShape(physx::PxShape *pShape) {
	physx::PxRigidActor *actor = pShape->getActor();
	glm::vec4 color = getColor(pShape->getActor(), DefaultDynamicRigidBodyCapsuleColor);

	physx::PxTransform pT = physx::PxShapeExt::getGlobalPose(*pShape, *actor);
	physx::PxCapsuleGeometry cg;
	pShape->getCapsuleGeometry(cg);
	physx::PxMat33 m = physx::PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 multm = gCamera.mvp * mat;

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);

	glm::mat4 rotation = glm::rotate(multm, Deg2Rad(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 translation0 = glm::translate(rotation, glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 scaled0 = glm::scale(translation0, glm::vec3(cg.radius, cg.radius, 2.0f * cg.halfHeight));
	gRenderer->LoadMatrix(scaled0);
	DrawPrimitive(gCylinderVBO, false);

	glm::mat4 translation1 = glm::translate(rotation, glm::vec3(0.0f, 0.0f, -cg.halfHeight));
	glm::mat4 scaled1 = glm::scale(translation1, glm::vec3(cg.radius));
	gRenderer->LoadMatrix(scaled1);
	DrawPrimitive(gSphereVBO, false);

	glm::mat4 translation2 = glm::translate(rotation, glm::vec3(0.0f, 0.0f, cg.halfHeight));
	glm::mat4 scaled2 = glm::scale(translation2, glm::vec3(cg.radius));
	gRenderer->LoadMatrix(scaled2);
	DrawPrimitive(gSphereVBO, false);

	gLightingShader->disable();
}

void DrawShape(physx::PxShape *shape) {
	physx::PxGeometryType::Enum type = shape->getGeometryType();

	switch(type) {
		case physx::PxGeometryType::eBOX:
			DrawBoxShape(shape);
			break;

		case physx::PxGeometryType::eSPHERE:
			DrawSphereShape(shape);
			break;

		case physx::PxGeometryType::eCAPSULE:
			DrawCapsuleShape(shape);
			break;
	}
}

void DrawBounds(const physx::PxBounds3 &bounds) {
	physx::PxVec3 center = bounds.getCenter();
	physx::PxVec3 scale = bounds.getDimensions();

	GLfloat mat_diffuse[4] = { 0, 1, 1, 1 };
	glColor4fv(mat_diffuse);

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(center.x, center.y, center.z));
	glm::mat4 scaled = glm::scale(translation, glm::vec3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f));
	glm::mat4 mvp = gCamera.mvp * scaled;
	gRenderer->LoadMatrix(mvp);
	DrawPrimitive(gBoxVBO, true);
}

void DrawActorBounding(physx::PxActor *actor) {
	physx::PxBounds3 bounds = actor->getWorldBounds();
	glm::vec3 min = toGLMVec3(bounds.minimum);
	glm::vec3 max = toGLMVec3(bounds.maximum);
	if(gFrustum.containsBounds(min, max)) {
		gDrawedActors++;
		DrawBounds(bounds);
	}
}

void DrawActor(physx::PxActor *actor) {
	physx::PxBounds3 bounds = actor->getWorldBounds();
	glm::vec3 min = toGLMVec3(bounds.minimum);
	glm::vec3 max = toGLMVec3(bounds.maximum);
	if(gFrustum.containsBounds(min, max)) {
		bool isVisible = true;
		bool blending = false;

		if(actor->userData) {
			Actor *a = (Actor *)actor->userData;
			isVisible = a->visible;
			blending = a->blending;
		}

		if(isVisible) {
			gDrawedActors++;

			physx::PxType actorType = actor->getConcreteType();
			if(actorType == physx::PxConcreteType::eRIGID_STATIC || actorType == physx::PxConcreteType::eRIGID_DYNAMIC) {
				physx::PxRigidActor *rigActor = (physx::PxRigidActor *)actor;
				physx::PxU32 nShapes = rigActor->getNbShapes();
				physx::PxShape **shapes = new physx::PxShape * [nShapes];

				if(blending) {
					gRenderer->SetBlending(true);
					glDisable(GL_DEPTH_TEST);
				}

				rigActor->getShapes(shapes, nShapes);

				while(nShapes--) {
					if((gHideRigidBodies == HideRigidBody_None) || (gHideRigidBodies == HideRigidBody_Blending && !blending))
						DrawShape(shapes[nShapes]);
				}

				delete[] shapes;

				if(blending) {
					gRenderer->SetBlending(false);
					glEnable(GL_DEPTH_TEST);
				}
			}
		}
	}
}

void RenderActors() {
	// Render all the actors in the scene
	for(size_t index = 0, count = gActors.size(); index < count; ++index) {
		Actor *actor = gActors[index];
		if(actor->physicsData != nullptr) {
			physx::PxActor *nactor = static_cast<physx::PxActor *>(actor->physicsData);
			DrawActor(nactor);
		}
	}
}

void RenderActorBoundings() {
	// Render all the actors in the scene as bounding volume
	for(size_t index = 0, count = gActors.size(); index < count; ++index) {
		Actor *actor = gActors[index];
		if(actor->physicsData != nullptr) {
			physx::PxActor *nactor = static_cast<physx::PxActor *>(actor->physicsData);
			DrawActorBounding(nactor);
		}
	}
}

void ShutdownPhysX() {
	ClearScene(*gScene);
	gScene->release();

	gDefaultMaterial->release();

	if(gCudaContextManager)
		gCudaContextManager->release();

#ifdef PVD_ENABLED
	if(gPhysXVisualDebugger != nullptr) {
		if(gPhysXVisualDebugger->isConnected())
			gPhysXVisualDebugger->disconnect();
		gPhysXVisualDebugger->release();
	}
#endif

	gPhysicsSDK->release();
	gPhysXFoundation->release();
}

const char *GetNameOfActorCreationKind(const ActorCreationKind kind) {
	switch(kind) {
		case ActorCreationKind::RigidBox:
			return "Rigid/Box\0";

		case ActorCreationKind::RigidSphere:
			return "Rigid/Sphere\0";

		case ActorCreationKind::RigidCapsule:
			return "Rigid/Capsule\0";

		case ActorCreationKind::FluidDrop:
			return "Fluid/Drop\0";

		case ActorCreationKind::FluidPlane:
			return "Fluid/Plane\0";

		case ActorCreationKind::FluidCube:
			return "Fluid/Cube\0";

		case ActorCreationKind::FluidSphere:
			return "Fluid/Sphere\0";

		default:
			return "Unknown\0";
	}
}

const char *GetDrawRigidbodyStr(unsigned int value) {
	switch(value) {
		case HideRigidBody_None:
			return "Yes\0";

		case HideRigidBody_Blending:
			return "Yes, except blending ones\0";

		case HideRigidBody_All:
			return "No\0";

		default:
			return "Unknown\0";
	}
}

const char *GetFluidProperty(const FluidProperty prop) {
	switch(prop) {
		case FluidProperty::Viscosity:
			return "Viscosity\0";

		case FluidProperty::Stiffness:
			return "Stiffness\0";

		case FluidProperty::MaxMotionDistance:
			return "Max motion distance\0";

		case FluidProperty::ContactOffset:
			return "Contact offset\0";

		case FluidProperty::RestOffset:
			return "Rest offset\0";

		case FluidProperty::Restitution:
			return "Restitution\0";

		case FluidProperty::Damping:
			return "Damping\0";

		case FluidProperty::DynamicFriction:
			return "Dynamic friction\0";

		case FluidProperty::ParticleMass:
			return "Particle mass\0";

		case FluidProperty::DepthBlurScale:
			return "Depth blur scale\0";

		case FluidProperty::ParticleRenderFactor:
			return "Particle render factor\0";

		case FluidProperty::DebugType:
			return "Debug type\0";

		case FluidProperty::ColorFalloffScale:
			return "Color falloff scale\0";

		case FluidProperty::ColorFalloffAlpha:
			return "Color falloff alpha\0";

		default:
			return "None\0";
	}
}

const char *GetFluidRenderMode(const SSFRenderMode mode) {
	switch(mode) {
		case SSFRenderMode::Disabled:
			return "Disabled\0";

		case SSFRenderMode::Fluid:
			return "Fluid\0";

		case SSFRenderMode::PointSprites:
			return "Point Sprites\0";

		case SSFRenderMode::Points:
			return "Points\0";

		default:
			return "None\0";
	}
}

const char *GetFluidDebugType(const FluidDebugType type) {
	switch(type) {
		case FluidDebugType::Final:
			return "Final\0";

		case FluidDebugType::Depth:
			return "Depth only\0";

		case FluidDebugType::Normal:
			return "Normal only\0";

		case FluidDebugType::Color:
			return "Color only\0";

		case FluidDebugType::Diffuse:
			return "Diffuse only\0";

		case FluidDebugType::Specular:
			return "Specular only\0";

		case FluidDebugType::DiffuseSpecular:
			return "Diffuse + Specular\0";

		case FluidDebugType::Scene:
			return "Scene only\0";

		case FluidDebugType::Fresnel:
			return "Fresne onlyl\0";

		case FluidDebugType::Refraction:
			return "Refraction only\0";

		case FluidDebugType::Reflection:
			return "Reflection only\0";

		case FluidDebugType::FresnelReflection:
			return "Fresnel + Reflection\0";

		case FluidDebugType::Thickness:
			return "Thickness only\0";

		case FluidDebugType::Absorbtion:
			return "Color absorption only\0";

		default:
			return "Unknown\0";
	}
}

void CreateActorsBasedOnTime(const float frametime) {
	// Add not fallen fluids from active scenario

	// Add actors
	for(size_t i = 0, count = gActors.size(); i < count; i++) {
		Actor *actor = gActors[i];
		if(actor->type != ActorType::Fluid) {
			if(actor->time > 0) {
				if(actor->timeElapsed < (float)actor->time) {
					actor->timeElapsed += frametime;
					if(actor->timeElapsed >= (float)actor->time) {
						AddScenarioActor(*gScene, actor);
					}
				}
			}
		}
	}

	// Add fluids
	for(size_t i = 0, count = gActors.size(); i < count; i++) {
		Actor *actor = gActors[i];
		if(actor->type != ActorType::Fluid) continue;

		FluidActor *fluid = static_cast<FluidActor *>(actor);

		float time;

		if(!fluid->isEmitter) {
			// Einmaliger partikel emitter
			if(fluid->time > 0) {
				time = (float)fluid->time;

				if(fluid->timeElapsed < time) {
					fluid->timeElapsed += frametime;

					if(fluid->timeElapsed >= time) {
						AddFluid(*gFluidSystem, *fluid, fluid->fluidType);
					}
				}
			}
		} else if(!gStoppedEmitter) {
			time = fluid->emitterTime;
			float duration = (float)fluid->emitterDuration;

			if(time > 0.0f) {
				fluid->emitterElapsed += frametime;

				if((fluid->emitterElapsed < duration) || (fluid->emitterDuration == 0)) {
					if(fluid->timeElapsed < time) {
						fluid->timeElapsed += frametime;

						if(fluid->timeElapsed >= time) {
							fluid->timeElapsed = 0.0f;
							AddFluid(*gFluidSystem, *fluid, fluid->fluidType);
						}
					}
				} else if(fluid->emitterCoolDown > 0.0f) {
					if(!fluid->emitterCoolDownActive) {
						fluid->emitterCoolDownActive = true;
						fluid->emitterCoolDownElapsed = 0.0f;
					}

					if(fluid->emitterCoolDownActive) {
						fluid->emitterCoolDownElapsed += frametime;

						if(fluid->emitterCoolDownElapsed >= (float)fluid->emitterCoolDown) {
							// Cool down finished
							fluid->emitterCoolDownActive = false;
							fluid->emitterElapsed = 0.0f;
							fluid->timeElapsed = 0.0f;
						}
					}
				}
			}
		}
	}
}

void Update(const glm::mat4 &proj, const glm::mat4 &modl, const float frametime) {
	// Update frustum
	gFrustum.update(&proj[0][0], &modl[0][0]);

	// Create actor based on time
	if(!gPaused) {
		CreateActorsBasedOnTime(frametime * 1000.0f);
	}

	// Update PhysX
	UpdatePhysX(frametime);
}

struct OSDRenderPosition {
	float x;
	float y;
	float fontHeight;
	float lineHeight;

	OSDRenderPosition(const float fontHeight, const float lineHeight):
		x(0),
		y(0),
		fontHeight(fontHeight),
		lineHeight(lineHeight) {
	}

	void newLine() {
		y += lineHeight;
	}
};

void RenderOSDLine(OSDRenderPosition &osdpos, char *value) {
	CTextureFont *font;
	if(osdpos.fontHeight <= 32)
		font = gFontTexture16;
	else
		font = gFontTexture32;


	gRenderer->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
	gRenderer->DrawString(0, font, osdpos.x, osdpos.y, osdpos.fontHeight, value);
	gRenderer->SetColor(1, 1, 1, 1);
	gRenderer->DrawString(0, font, osdpos.x + 1, osdpos.y + 1, osdpos.fontHeight, value);
	osdpos.newLine();
}

std::string drawingError = "";
void RenderOSD(const int windowWidth, const int windowHeight) {
	char buffer[256];

	// Setup ortho for font rendering
	glm::mat4 orthoProj = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
	glm::mat4 orthoMVP = glm::mat4(1.0f) * orthoProj;
	gRenderer->LoadMatrix(orthoMVP);

	// Disable depth testing
	gRenderer->SetDepthTest(false);

	// Enable blending
	gRenderer->SetBlending(true);

	if(gShowOSD) {
		// Draw background
		gRenderer->SetColor(0.1f, 0.1f, 0.1f, 0.2f);
		gRenderer->DrawSimpleRect(0.0f, 0.0f, (float)windowWidth * 0.25f, (float)windowHeight);
		gRenderer->SetColor(1, 1, 1, 1);
	}

	// Font height is proportional to window height
	const float targetFontScale = 0.0225f;
	float fontHeight = (float)windowHeight * targetFontScale;

	OSDRenderPosition osdPos = OSDRenderPosition(fontHeight, fontHeight * 0.9f);
	osdPos.x = 20;
	osdPos.y = 20;

	// Render text
	sprintf_s(buffer, "FPS: %3.2f", gFps);
	RenderOSDLine(osdPos, buffer);
	sprintf_s(buffer, "Show osd: %s (T)", gShowOSD ? "yes" : "no");
	RenderOSDLine(osdPos, buffer);

	if(gShowOSD) {
		sprintf_s(buffer, "Drawed actors: %zu of %zu", gDrawedActors, gTotalActors);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Total fluid particles: %lu", gTotalFluidParticles);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw error: %s", drawingError.c_str());
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Simulation state (O): %s", gPaused ? "paused" : "running");
		RenderOSDLine(osdPos, buffer);

		// Empty line
		osdPos.newLine();

		const FluidColor &activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);

		sprintf_s(buffer, "Controls:");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Geometry type (1-7): %s", GetNameOfActorCreationKind(gCurrentActorCreationKind));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Wireframe (W): %s", gDrawWireframe ? "enabled" : "disabled");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Boundbox (B): %s", gDrawBoundBox ? "enabled" : "disabled");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Rigidbodies (D): %s", GetDrawRigidbodyStr(gHideRigidBodies));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid Rendering Mode (S): %s", GetFluidRenderMode(gSSFRenderMode));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid color (C): %d / %zu - %s", gSSFCurrentFluidIndex + 1, gActiveScene->getFluidColorCount(), activeFluidColor.name);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid detail level (P): %3.2f %s", gSSFDetailFactor * 100.0f, "%");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid blur depth active: (M): %s", gSSFBlurActive ? "yes" : "no");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid current property (V): %s", GetFluidProperty(gFluidCurrentProperty));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid viscosity: %f", gFluidViscosity);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid stiffness: %f", gFluidStiffness);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid rest offset: %f", gFluidRestOffset);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid contact offset: %f", gFluidContactOffset);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid restitution: %f", gFluidRestitution);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid damping: %f", gFluidDamping);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid dynamic friction: %f", gFluidDynamicFriction);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid particle mass: %f", gFluidParticleMass);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid max motion distance: %f", gFluidMaxMotionDistance);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid blur depth scale: %f", gSSFBlurDepthScale);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid particle render factor: %f", gFluidParticleRenderFactor);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid debug type: %d / %d (%s)", (int)gFluidDebugType, FluidDebugType::Max, GetFluidDebugType(gFluidDebugType));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid color falloff scale: %f", activeFluidColor.falloffScale);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid color falloff alpha: %f", activeFluidColor.falloff.w);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid scenario (L): %d / %zu - %s", gActiveFluidScenarioIdx + 1, gFluidScenarios.size(), gActiveFluidScenario ? gActiveFluidScenario->displayName : "No scenario loaded!");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "New actor (Space)");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Reset current scene (R)");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid add acceleration (Arrow Keys)");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid using GPU acceleration (H): %s", gFluidUseGPUAcceleration ? "yes" : "no");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid emitter active (K): %s", !gStoppedEmitter ? "yes" : "no");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Add fluid after scene change (N): %s", !gWaterAddBySceneChange ? "yes" : "no");
		RenderOSDLine(osdPos, buffer);

		// Empty line
		osdPos.newLine();

		sprintf_s(buffer, "Fluid particle radius: %f", gFluidParticleRadius);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid rest particle distance: %f", gFluidRestParticleDistance);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid cell size: %f", gFluidCellSize);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid min density: %f", gActiveFluidScenario ? gActiveFluidScenario->render.minDensity : gActiveScene->render.minDensity);
		RenderOSDLine(osdPos, buffer);
	}

	// Disable blending
	gRenderer->SetBlending(false);

	// Enable depth testing
	gRenderer->SetDepthTest(true);
}

void RenderSkybox(const glm::mat4 &mvp) {
	if(!gSkyboxVBO) return;
	if(!gSkyboxShader) return;
	if(!gSkyboxCubemap) return;

	gRenderer->SetColor(1, 1, 1, 1);

	gRenderer->SetDepthMask(false);

	gRenderer->EnableTexture(0, gSkyboxCubemap);

	gSkyboxShader->enable();
	gSkyboxShader->uniformMatrix4(gSkyboxShader->ulocMVP, &mvp[0][0]);
	gSkyboxShader->uniform1i(gSkyboxShader->ulocCubemap, 0);
	DrawPrimitive(gSkyboxVBO, false);
	gSkyboxShader->disable();

	gRenderer->DisableTexture(0, gSkyboxCubemap);

	gRenderer->SetDepthMask(true);
}

void RenderScene(const glm::mat4 &mvp) {
	// Draw skybox
	RenderSkybox(mvp);

	// Draw the grid
	DrawGrid(40);

	if(gDrawWireframe)
		gRenderer->SetWireframe(true);

	// Render actors
	RenderActors();

	// Render actor boundings if required
	if(gDrawBoundBox) {
		if(!gDrawWireframe)
			gRenderer->SetWireframe(true);

		RenderActorBoundings();

		if(!gDrawWireframe)
			gRenderer->SetWireframe(false);
	}

	if(gDrawWireframe)
		gRenderer->SetWireframe(false);
}

void RenderSceneFBO(const glm::mat4 &mvp, const int windowWidth, const int windowHeight) {
	// Save latest draw buffer
	GLint latestDrawBuffer = gSceneFBO->getDrawBuffer();

	gRenderer->SetViewport(0, 0, windowWidth, windowHeight);
	gRenderer->LoadMatrix(mvp);

	// Resize FBO if required
	if(gSceneFBO->getWidth() != windowWidth || gSceneFBO->getHeight() != windowHeight) {
		gSceneFBO->resize(windowWidth, windowHeight);
	}

	// Enable FBO
	gSceneFBO->enable();
	gSceneFBO->setDrawBuffer(GL_COLOR_ATTACHMENT0);
	gRenderer->Clear(ClearFlags::Color | ClearFlags::Depth);

	// Render scene
	RenderScene(mvp);

	// Enable FBO
	gSceneFBO->disable();

	// Restore latest draw buffer
	gSceneFBO->setDrawBuffer(latestDrawBuffer);

	gRenderer->SetViewport(0, 0, windowWidth, windowHeight);
	gRenderer->LoadMatrix(mvp);
}

void OnRender(const int windowWidth, const int windowHeight, const float frametime) {
	if(!gScene) return;

	float realFrametimeStart = (float)fplGetTimeInMillisecondsHP();

	// TODO(final): Revisit any time / delta computation, because its not correct
	gTotalFrames++;
	if((realFrametimeStart - gAppStartTime) > 1000.0f) {
		float elapsedTime = float(realFrametimeStart - gAppStartTime);
		gFps = (((float)gTotalFrames * 1000.0f) / elapsedTime);
		gAppStartTime = realFrametimeStart;
		gTotalFrames = 0;
	}

	// Update counters
	gTotalActors = gActors.size();
	gDrawedActors = 0;

	// Set drawing options
	const FluidColor &activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);;
	SSFDrawingOptions options = SSFDrawingOptions();
	options.textureState = 0;
	options.renderMode = gSSFRenderMode;
	options.fluidColor = activeFluidColor;
	options.blurScale = gSSFBlurDepthScale;
	options.blurEnabled = gSSFBlurActive;
	options.debugType = gFluidDebugType;

	// Set viewport
	gRenderer->SetViewport(0, 0, windowWidth, windowHeight);
	gRenderer->SetScissor(0, 0, windowWidth, windowHeight);

	// Create ortho projections
	glm::mat4 orthoProj = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
	glm::mat4 orthoMVP = glm::mat4(1.0f) * orthoProj;

	// Create camera
	gCamera = CCamera(0.0f, 4.0f, gCameraDistance, Deg2Rad(gCamRotation.x), Deg2Rad(gCamRotation.y), DefaultZNear, DefaultZFar, Deg2Rad(DefaultFov), (float)windowWidth / (float)windowHeight);
	glm::mat4 mvp = gCamera.mvp;
	glm::mat4 proj = gCamera.projection;
	glm::mat4 mdlv = gCamera.modelview;
	gRenderer->LoadMatrix(mvp);

	// Update (Frustum and PhysX)
	Update(proj, mdlv, frametime);

	// Clear back buffer
	glm::vec3 backcolor = gActiveScene->backgroundColor;
	gRenderer->ClearColor(backcolor.x, backcolor.y, backcolor.z, 0.0f);
	gRenderer->Clear(ClearFlags::Color | ClearFlags::Depth);

	bool drawFluidParticles = gSSFRenderMode != SSFRenderMode::Disabled;

	// Render scene to FBO
	if(drawFluidParticles)
		RenderSceneFBO(mvp, windowWidth, windowHeight);

	// Load mvp matrix
	gRenderer->LoadMatrix(mvp);

	// Render scene
	gDrawedActors = 0;
	if(!drawFluidParticles || options.debugType == FluidDebugType::Final)
		RenderScene(mvp);

	// Render fluid
	if(drawFluidParticles) {
		gFluidRenderer->setParticleRadius(gFluidParticleRadius * gFluidParticleRenderFactor);
		gFluidRenderer->Render(gCamera, gTotalFluidParticles, options, windowWidth, windowHeight);
	}

	// Check for opengl error
	drawingError = gRenderer->CheckError();

	// Render OSD
	RenderOSD(windowWidth, windowHeight);

	// Draw frame
	gRenderer->Flip();

	float curTime = (float)fplGetTimeInMillisecondsHP();
	gTotalTimeElapsed += (curTime - realFrametimeStart);
}

enum class MouseAction: int {
	None = 0,
	Rotate,
	Zoom,
};

static fplMouseButtonType gMouseButton = fplMouseButtonType::fplMouseButtonType_None;
static bool gMouseDown = false;
static MouseAction gMouseAction = MouseAction::None;
static int gMouseOldX = -1;
static int gMouseOldY = -1;

static void OnMouseButton(const fplMouseButtonType button, fplButtonState s, int x, int y) {
	if(s == fplButtonState::fplButtonState_Press) {
		gMouseOldX = x;
		gMouseOldY = y;
		if(button == fplMouseButtonType::fplMouseButtonType_Left)
			gMouseAction = MouseAction::Rotate;
		else if(button == fplMouseButtonType::fplMouseButtonType_Right)
			gMouseAction = MouseAction::Zoom;
		else
			gMouseAction = MouseAction::None;
		gMouseButton = button;
		gMouseDown = true;
	} else if(s == fplButtonState::fplButtonState_Release) {
		gMouseOldX = x;
		gMouseOldY = y;
		gMouseAction = MouseAction::None;
		gMouseDown = false;
	}
}

void OnMouseMove(const fplMouseButtonType button, const fplButtonState state, const int x, const int y) {
	if(gMouseDown) {
		if(gMouseAction == MouseAction::Zoom) {
			gCameraDistance *= (1 + (y - gMouseOldY) / 60.0f);
		} else if(gMouseAction == MouseAction::Rotate) {
			gCamRotation.y += (x - gMouseOldX) / 5.0f;
			gCamRotation.x += (y - gMouseOldY) / 5.0f;
		}
		gMouseOldX = x;
		gMouseOldY = y;
	}
}

static void AddDynamicActor(physx::PxScene &scene, CFluidSystem &fluidSys, const ActorCreationKind kind) {
	glm::vec3 pos = gRigidBodyFallPos;
	glm::vec3 vel = DefaultRigidBodyVelocity;
	float density = DefaultRigidBodyDensity;
	glm::quat rotation = toGLMQuat(physx::PxQuat(Deg2Rad(RandomRadius()), physx::PxVec3(0, 1, 0)));
	switch(kind) {
		case ActorCreationKind::RigidBox:
		{
			CubeActor *box = new CubeActor(ActorMovementType::Dynamic, glm::vec3(0.5, 0.5, 0.5));
			box->color = glm::vec4(0.0f, 0.1f, 1.0f, 1.0f);
			box->transform.position = pos;
			box->transform.rotation = rotation;
			box->velocity = vel;
			box->density = density;
			AddBox(scene, box);
			gActors.push_back(box);
		} break;

		case ActorCreationKind::RigidSphere:
		{
			SphereActor *sphere = new SphereActor(ActorMovementType::Dynamic, 0.5f);
			sphere->color = glm::vec4(0.0f, 1.0f, 0.1f, 1.0f);
			sphere->transform.position = pos;
			sphere->transform.rotation = rotation;
			sphere->velocity = vel;
			sphere->density = density;
			AddSphere(scene, sphere);
			gActors.push_back(sphere);
		} break;

		case ActorCreationKind::RigidCapsule:
		{
			CapsuleActor *capsule = new CapsuleActor(ActorMovementType::Dynamic, 0.5f, 0.5f);
			capsule->color = glm::vec4(1.0f, 0.9f, 0.1f, 1.0f);
			capsule->transform.position = pos;
			capsule->transform.rotation = rotation;
			capsule->velocity = vel;
			capsule->density = density;
			AddCapsule(scene, capsule);
			gActors.push_back(capsule);
		} break;

		case ActorCreationKind::FluidDrop:
			AddFluids(fluidSys, FluidType::Drop);
			break;

		case ActorCreationKind::FluidPlane:
			AddFluids(fluidSys, FluidType::Plane);
			break;

		case ActorCreationKind::FluidCube:
			AddFluids(fluidSys, FluidType::Box);
			break;

		case ActorCreationKind::FluidSphere:
			AddFluids(fluidSys, FluidType::Sphere);
			break;

		default:
			break;
	}
}

void ToggleFluidGPUAcceleration() {
	if(gGPUDispatcher) {
		physx::PxActor *fluidActor = gFluidSystem->getActor();
		gFluidUseGPUAcceleration = !gFluidUseGPUAcceleration;
		gScene->removeActor(*fluidActor);
		gFluidSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, gFluidUseGPUAcceleration);
		gScene->addActor(*fluidActor);
	}
}

void SetFluidExternalAcceleration(const physx::PxVec3 &acc) {
	gFluidLatestExternalAccelerationTime = fplGetTimeInMillisecondsLP() + 3000; // 3 Seconds
	gFluidSystem->setExternalAcceleration(acc);
}

void KeyUp(const fplKey key, const int x, const int y) {
	switch(key) {
		case fplKey_Escape: // Escape
		{
			fplWindowShutdown();
			break;
		}



		case fplKey_1: // 1 - 7
		case fplKey_2:
		case fplKey_3:
		case fplKey_4:
		case fplKey_5:
		case fplKey_6:
		case fplKey_7:
		{
			int index = (int)(key - fplKey_1);
			assert(index >= 0 && index <= (int)ActorCreationKind::Max);
			gCurrentActorCreationKind = (ActorCreationKind)index;
			break;
		}
		case fplKey_F: // f
		{
			bool wasFullscreen = fplIsWindowFullscreen();
			fplSetWindowFullscreenSize(!wasFullscreen, 0, 0, 0);
			break;
		}
		case fplKey_R: // r
		{
			ResetScene(*gScene);
			break;
		}

		case fplKey_T: // t
		{
			gShowOSD = !gShowOSD;
			break;
		}

		case fplKey_B: // b
		{
			gDrawBoundBox = !gDrawBoundBox;
			break;
		}

		case fplKey_D: // d
		{
			gHideRigidBodies++;

			if(gHideRigidBodies > HideRigidBody_MAX)
				gHideRigidBodies = 0;

			break;
		}

		case fplKey_W: // w
		{
			gDrawWireframe = !gDrawWireframe;
			break;
		}

		case fplKey_V: // v
		{
			int index = (int)gFluidCurrentProperty;
			index++;
			if(index > (int)FluidProperty::Last) index = (int)FluidProperty::None;
			gFluidCurrentProperty = (FluidProperty)index;

			break;
		}

		case fplKey_H: // h
		{
			ToggleFluidGPUAcceleration();
			break;
		}

		case fplKey_K: // k
		{
			gStoppedEmitter = !gStoppedEmitter;
			break;
		}

		case fplKey_L: // l
		{
			if(gFluidScenarios.size() > 0) {
				gActiveFluidScenarioIdx++;

				if(gActiveFluidScenarioIdx > (int)gFluidScenarios.size() - 1) gActiveFluidScenarioIdx = 0;

				gActiveFluidScenario = gFluidScenarios[gActiveFluidScenarioIdx];
				ResetScene(*gScene);
			}

			break;
		}

		case fplKey_M: // m
		{
			gSSFBlurActive = !gSSFBlurActive;
			break;
		}

		case fplKey_N: // n
		{
			gWaterAddBySceneChange = !gWaterAddBySceneChange;
			break;
		}

		case fplKey_S: // s
		{
			gFluidDebugType = FluidDebugType::Final;

			int mode = (int)gSSFRenderMode;

			mode++;
			if(mode > (int)SSFRenderMode::Disabled) mode = (int)SSFRenderMode::Fluid;
			gSSFRenderMode = (SSFRenderMode)mode;

			SingleStepPhysX(gPhysicsAccumulator, InitPhysicsDT);
			break;
		}

		case fplKey_C: // c
		{
			gSSFCurrentFluidIndex++;

			if(gSSFCurrentFluidIndex > (int)gActiveScene->getFluidColorCount() - 1) gSSFCurrentFluidIndex = 0;

			break;
		}

		case fplKey_O: // o
		{
			gPaused = !gPaused;
			break;
		}

		case fplKey_P: // p
		{
			gSSFDetailFactor += -0.10f;

			if(gSSFDetailFactor < 0.0f) gSSFDetailFactor = 1.0f;

			if(gFluidRenderer)
				gFluidRenderer->SetFBOFactor(gSSFDetailFactor);

			break;
		}

		default:
			break;
	}
}

float roundFloat(float x) {
	const float sd = 1000; //for accuracy to 3 decimal places
	return int(x * sd + (x < 0 ? -0.5 : 0.5)) / sd;
}

void ChangeFluidProperty(float value) {
	if(!gActiveFluidScenario) return;

	switch(gFluidCurrentProperty) {
		case FluidProperty::Viscosity:
		{
			gFluidViscosity += value;
			gFluidSystem->setViscosity(gFluidViscosity);
			gActiveFluidScenario->sim.viscosity = gFluidViscosity;
		} break;

		case FluidProperty::Stiffness:
		{
			gFluidStiffness += value;
			gFluidSystem->setStiffness(gFluidStiffness);
			gActiveFluidScenario->sim.stiffness = gFluidStiffness;
		} break;

		case FluidProperty::MaxMotionDistance:
		{
			gFluidMaxMotionDistance += value / 1000.0f;
			gFluidSystem->setMaxMotionDistance(gFluidMaxMotionDistance);
			gActiveScene->sim.maxMotionDistance = gFluidMaxMotionDistance;
		} break;

		case FluidProperty::ContactOffset:
		{
			gFluidContactOffset += value / 1000.0f;
			gFluidSystem->setContactOffset(gFluidContactOffset);
			gActiveScene->sim.contactOffset = gFluidContactOffset;
		} break;

		case FluidProperty::RestOffset:
		{
			gFluidRestOffset += value / 1000.0f;
			gFluidSystem->setRestOffset(gFluidRestOffset);
			gActiveScene->sim.restOffset = gFluidRestOffset;
		} break;

		case FluidProperty::Restitution:
		{
			gFluidRestitution += value / 1000.0f;
			gFluidSystem->setRestitution(gFluidRestitution);
			gActiveScene->sim.restitution = gFluidRestitution;
		} break;

		case FluidProperty::Damping:
		{
			gFluidDamping += value / 1000.0f;
			gFluidSystem->setDamping(gFluidDamping);
			gActiveScene->sim.damping = gFluidDamping;
		} break;

		case FluidProperty::DynamicFriction:
		{
			gFluidDynamicFriction += value / 1000.0f;
			gFluidSystem->setDynamicFriction(gFluidDynamicFriction);
			gActiveScene->sim.dynamicFriction = gFluidDynamicFriction;
		} break;

		case FluidProperty::StaticFriction:
		{
			gFluidStaticFriction += value / 1000.0f;
			gFluidSystem->setStaticFriction(gFluidStaticFriction);
			gActiveScene->sim.staticFriction = gFluidStaticFriction;
		} break;

		case FluidProperty::ParticleMass:
		{
			gFluidParticleMass += value / 1000.0f;
			gFluidSystem->setParticleMass(gFluidParticleMass);
			gActiveScene->sim.particleMass = gFluidParticleMass;
		} break;

		case FluidProperty::DepthBlurScale:
		{
			gSSFBlurDepthScale += value / 10000.0f;
		} break;

		case FluidProperty::ParticleRenderFactor:
		{
			gFluidParticleRenderFactor += value / 10.0f;
			gFluidParticleRenderFactor = roundFloat(gFluidParticleRenderFactor);
		} break;

		case FluidProperty::DebugType:
		{
			int inc = (int)value;

			int debugType = (int)gFluidDebugType;
			debugType += inc;

			if(debugType < 0) debugType = (int)FluidDebugType::Max;
			if(debugType > (int)FluidDebugType::Max) debugType = (int)FluidDebugType::Final;

			gFluidDebugType = (FluidDebugType)debugType;
		} break;

		case FluidProperty::ColorFalloffScale:
		{
			FluidColor &activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);
			activeFluidColor.falloffScale += value / 100.0f;
		} break;

		case FluidProperty::ColorFalloffAlpha:
		{
			FluidColor &activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);
			activeFluidColor.falloff.w += value / 100.0f;
		} break;
	}
}

void KeyDown(unsigned char key, int x, int y) {
	const float accSpeed = 10.0f;
	const physx::PxForceMode::Enum accMode = physx::PxForceMode::eACCELERATION;

	switch(key) {
		case fplKey_Right:
		{
			gFluidSystem->addForce(physx::PxVec3(1.0f * accSpeed, 0.0f, 0.0f), accMode);
			break;
		}

		case fplKey_Left:
		{
			gFluidSystem->addForce(physx::PxVec3(-1.0f * accSpeed, 0.0f, 0.0f), accMode);
			break;
		}

		case fplKey_Up:
		{
			gFluidSystem->addForce(physx::PxVec3(0.0f, 0.0f, -1.0f * accSpeed), accMode);
			break;
		}

		case fplKey_Down:
		{
			gFluidSystem->addForce(physx::PxVec3(0.0f, 0.0f, 1.0f * accSpeed), accMode);
			break;
		}

		case fplKey_Space: // Space
		{
			AddDynamicActor(*gScene, *gFluidSystem, gCurrentActorCreationKind);
			break;
		}

		case fplKey_Add: // +
		{
			// Increase fluid property
			ChangeFluidProperty(1.0f);
			break;
		}

		case fplKey_Substract: // -
		{
			// Decrease fluid property
			ChangeFluidProperty(-1.0f);
			break;
		}

		default:
			break;
	}
}

void LoadFluidScenarios(const char *appPath) {
	// Load scenarios
	std::string scenariosPath = COSLowLevel::pathCombine(appPath, "scenarios");
	std::vector<std::string> scenFiles = COSLowLevel::getFilesInDirectory(scenariosPath.c_str(), "*.xml");

	for(unsigned int i = 0; i < scenFiles.size(); i++) {
		std::string filePath = COSLowLevel::pathCombine(scenariosPath, scenFiles[i]);
		Scenario *scenario = Scenario::load(filePath.c_str(), gActiveScene);
		gFluidScenarios.push_back(scenario);
	}

	if(gFluidScenarios.size() > 0) {
		gActiveFluidScenarioIdx = 0;
		gActiveFluidScenario = gFluidScenarios[gActiveFluidScenarioIdx];
	} else {
		gActiveFluidScenarioIdx = -1;
		gActiveFluidScenario = nullptr;
		std::cerr << "  No fluid scenario found!" << std::endl;
	}
}

void printOpenGLInfos() {
	printf("  OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("  OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("  OpenGL Version: %s\n", glGetString(GL_VERSION));

#if 0
	if(GLEW_VERSION_2_0)
		printf("  GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if(glewIsSupported("GL_ARB_framebuffer_object")) {
		int temp;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &temp);
		printf("  OpenGL FBO Max Color Attachments: %d\n", temp);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &temp);
		printf("  OpenGL FBO Max Render Buffer Size: %d\n", temp);
	}
#endif
}

static void InitResources(const char *appPath) {
	// Create texture manager
	printf("  Create texture manager\n");
	gTexMng = new CTextureManager();

	gSkyboxCubemap = gTexMng->addCubemap("skybox", "textures\\skybox_texture.jpg");

	gFontAtlas16 = FontAtlas::LoadFromMemory(sulphurPointRegularData, 0, 16.0f, 32, 255);
	gFontAtlas32 = FontAtlas::LoadFromMemory(sulphurPointRegularData, 0, 32.0f, 32, 255);
	gFontTexture16 = gTexMng->addFont("Font", *gFontAtlas16);
	gFontTexture32 = gTexMng->addFont("Font", *gFontAtlas32);

	// Create scene
	printf("  Load scene\n");
	gActiveScene = new CScene(
		FluidSimulationProperties::DefaultParticleRadius,
		FluidSimulationProperties::DefaultViscosity,
		FluidSimulationProperties::DefaultStiffness,
		FluidSimulationProperties::DefaultParticleRestDistanceFactor,
		FluidRenderProperties::DefaultParticleRenderFactor,
		FluidRenderProperties::DefaultMinDensity,
		DefaultRigidBodyDensity);
	gActiveScene->load("scene.xml");
	gFluidParticleRadius = gActiveScene->sim.particleRadius;
	gFluidCellSize = gActiveScene->sim.cellSize;
	gFluidParticleRenderFactor = gActiveScene->render.particleRenderFactor;
	gSSFCurrentFluidIndex = gActiveScene->fluidColorDefaultIndex;

	// Create spherical point sprites
	printf("  Allocate spherical point sprites\n");
	gPointSprites = new CSphericalPointSprites();
	gPointSprites->Allocate(MaxFluidParticleCount);

	// Create spherical point sprites shader
	printf("  Load spherical point sprites shader\n");
	gPointSpritesShader = new CPointSpritesShader();
	Utils::attachShaderFromFile(gPointSpritesShader, GL_VERTEX_SHADER, "shaders\\PointSprites.vertex", "    ");
	Utils::attachShaderFromFile(gPointSpritesShader, GL_FRAGMENT_SHADER, "shaders\\PointSprites.fragment", "    ");

	// Create scene FBO
	printf("  Create scene FBO\n");
	gSceneFBO = new CSceneFBO(128, 128); // Initial FBO size does not matter, because its resized on render anyway
	gSceneFBO->depthTexture = gSceneFBO->addRenderTarget(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, GL_DEPTH_ATTACHMENT, GL_NEAREST);
	gSceneFBO->sceneTexture = gSceneFBO->addTextureTarget(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0, GL_LINEAR);
	gSceneFBO->update();

	// Create scene shader
	printf("  Create ortho shader\n");
	gSceneShader = new CGLSL();
	Utils::attachShaderFromFile(gSceneShader, GL_VERTEX_SHADER, "shaders\\Ortho.vertex", "    ");
	Utils::attachShaderFromFile(gSceneShader, GL_FRAGMENT_SHADER, "shaders\\Ortho.fragment", "    ");

	// Create fluid renderer
	printf("  Create fluid renderer\n");
	gFluidRenderer = new CScreenSpaceFluidRendering(128, 128, gFluidParticleRadius * 2.0f); // Initial FBO size does not matter, because its resized on render anyway
	gFluidRenderer->SetRenderer(gRenderer);
	gFluidRenderer->SetPointSprites(gPointSprites);
	gFluidRenderer->SetPointSpritesShader(gPointSpritesShader);
	gFluidRenderer->SetSceneTexture(gSceneFBO->sceneTexture);
	gFluidRenderer->SetSkyboxCubemap(gSkyboxCubemap);

	// Create lightung shader
	printf("  Create lighting renderer\n");
	gLightingShader = new CLightingShader();
	Utils::attachShaderFromFile(gLightingShader, GL_VERTEX_SHADER, "shaders\\Lighting.vertex", "    ");
	Utils::attachShaderFromFile(gLightingShader, GL_FRAGMENT_SHADER, "shaders\\Lighting.fragment", "    ");

	// Create skybox vbo and shader
	printf("  Create skybox\n");
	gSkyboxVBO = new GeometryVBO();
	{
		Primitives::Primitive skyboxPrim = Primitives::CreateBox(glm::vec3(100.0f), true);
		gSkyboxVBO->bufferVertices(skyboxPrim.verts[0].data(), skyboxPrim.sizeOfVertices, GL_STATIC_DRAW);
		gSkyboxVBO->bufferIndices(&skyboxPrim.indices[0], (GLuint)skyboxPrim.indexCount, GL_STATIC_DRAW);
		gSkyboxVBO->triangleIndexCount = skyboxPrim.indexCount;
	}
	gSkyboxShader = new CSkyboxShader();
	Utils::attachShaderFromFile(gSkyboxShader, GL_VERTEX_SHADER, "shaders\\Skybox.vertex", "    ");
	Utils::attachShaderFromFile(gSkyboxShader, GL_FRAGMENT_SHADER, "shaders\\Skybox.fragment", "    ");

	// Create geometry buffers
	printf("  Create geometry buffers\n");
	gBoxVBO = new GeometryVBO();
	{
		Primitives::Primitive prim = Primitives::CreateBox(glm::vec3(1.0f), false);
		gBoxVBO->bufferVertices(prim.verts[0].data(), prim.sizeOfVertices, GL_STATIC_DRAW);
		//gBoxVBO->bufferIndices(&geoBoxPrim.indices[0], (GLuint)geoBoxPrim.indexCount, GL_STATIC_DRAW);
		gBoxVBO->reserveIndices(prim.indexCount + prim.lineIndexCount, GL_STATIC_DRAW);
		gBoxVBO->subbufferIndices(&prim.indices[0], 0, prim.indexCount);
		gBoxVBO->subbufferIndices(&prim.lineIndices[0], prim.indexCount, prim.lineIndexCount);
		gBoxVBO->triangleIndexCount = prim.indexCount;
		gBoxVBO->lineIndexCount = prim.lineIndexCount;
	}
	gSphereVBO = new GeometryVBO();
	{
		Primitives::Primitive prim = Primitives::CreateSphere(1.0f, 16, 16);
		gSphereVBO->bufferVertices(prim.verts[0].data(), prim.sizeOfVertices, GL_STATIC_DRAW);
		gSphereVBO->reserveIndices(prim.indexCount + prim.lineIndexCount, GL_STATIC_DRAW);
		gSphereVBO->subbufferIndices(&prim.indices[0], 0, prim.indexCount);
		gSphereVBO->subbufferIndices(&prim.lineIndices[0], prim.indexCount, prim.lineIndexCount);
		gSphereVBO->triangleIndexCount = prim.indexCount;
		gSphereVBO->lineIndexCount = prim.lineIndexCount;

	}
	gCylinderVBO = new GeometryVBO();
	{
		Primitives::Primitive prim = Primitives::CreateCylinder(1.0f, 1.0f, 1.0f, 16, 16);
		gCylinderVBO->bufferVertices(prim.verts[0].data(), prim.sizeOfVertices, GL_STATIC_DRAW);
		gCylinderVBO->reserveIndices(prim.indexCount + prim.lineIndexCount, GL_STATIC_DRAW);
		gCylinderVBO->subbufferIndices(&prim.indices[0], 0, prim.indexCount);
		gCylinderVBO->subbufferIndices(&prim.lineIndices[0], prim.indexCount, prim.lineIndexCount);
		gCylinderVBO->triangleIndexCount = prim.indexCount;
		gCylinderVBO->lineIndexCount = prim.lineIndexCount;
	}
}

void ReleaseResources() {
	printf("  Release geometry buffers\n");
	if(gCylinderVBO != nullptr)
		delete gCylinderVBO;
	if(gSphereVBO != nullptr)
		delete gSphereVBO;
	if(gBoxVBO != nullptr)
		delete gBoxVBO;

	printf("  Release skybox\n");
	if(gSkyboxShader != nullptr)
		delete gSkyboxShader;
	if(gSkyboxVBO != nullptr)
		delete gSkyboxVBO;

	if(gLightingShader != nullptr)
		delete gLightingShader;

	printf("  Release fluid renderer\n");
	if(gFluidRenderer)
		delete gFluidRenderer;

	printf("  Release ortho shader\n");
	if(gSceneShader != nullptr)
		delete gSceneShader;

	// Release scene FBO
	printf("  Release scene fbo\n");

	if(gSceneFBO != nullptr)
		delete gSceneFBO;

	// Release point sprites shader
	printf("  Release spherical point sprites\n");

	if(gPointSpritesShader != nullptr)
		delete gPointSpritesShader;

	if(gPointSprites != nullptr)
		delete gPointSprites;

	// Release scene
	printf("  Release scene\n");

	if(gActiveScene != nullptr)
		delete gActiveScene;

	// Release texture manager
	printf("  Release texture manager\n");

	if(gTexMng != nullptr)
		delete gTexMng;

	if(gFontAtlas32 != nullptr) {
		delete gFontAtlas32;
	}
	if(gFontAtlas16 != nullptr) {
		delete gFontAtlas16;
	}
}

void OnShutdown() {
	// Release physx
	printf("Release PhysX\n");
	ShutdownPhysX();

	printf("Release Resources\n");
	ReleaseResources();

	// Release scenarios
	printf("Release Fluid Scenarios\n");

	for(unsigned int i = 0; i < gFluidScenarios.size(); i++) {
		Scenario *scen = gFluidScenarios[i];
		delete scen;
	}

	gFluidScenarios.clear();

	// Release renderer
	printf("Release Renderer\n");

	if(gRenderer)
		delete gRenderer;
}

int main(int argc, char **argv) {
	fplConsoleFormatOut("%s v%s\n", APPLICATION_NAME, APPLICATION_VERSION);
	fplConsoleFormatOut("%s\n", APPLICATION_COPYRIGHT);
	fplConsoleFormatOut("\n");

	// Get application path
	std::string appPath = COSLowLevel::getAppPath(argc, argv);

	// Initialize random generator
	srand((unsigned int)time(nullptr));

	// Initialize glut window
	fplConsoleFormatOut("Initialize Window\n");
	fplSettings platformSettings = fplMakeDefaultSettings();
	platformSettings.window.windowSize.width = DefaultWindowWidth;
	platformSettings.window.windowSize.height = DefaultWindowHeight;
	platformSettings.window.isFullscreen = false;
	platformSettings.video.isVSync = true;
	platformSettings.video.graphics.opengl.compabilityFlags = fplOpenGLCompabilityFlags_Legacy;
	platformSettings.video.graphics.opengl.majorVersion = 2;
	platformSettings.video.graphics.opengl.minorVersion = 1;
	fplCopyString(APPTITLE.c_str(), platformSettings.window.title, fplArrayCount(platformSettings.window.title));
	if(fplPlatformInit(fplInitFlags_Console | fplInitFlags_Video, &platformSettings)) {

		if(!gladLoadGL()) {
			std::cerr << "Failed to initialize OpenGL loader" << std::endl;
			fplPlatformRelease();
			return -1;
		}

		// Print OpenGL stuff
		printOpenGLInfos();

		// Required extensions check
		fplConsoleFormatOut("  Checking opengl requirements...");

		int maxColorAttachments = 0;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);

		if(!gladHasExtension("GL_ARB_texture_float") ||
			!gladHasExtension("GL_ARB_point_sprite") ||
			!gladHasExtension("GL_ARB_framebuffer_object")) {
			fplConsoleFormatError("failed\n");
			std::cerr << std::endl << "Your graphics adapter is not supported, press any key to exit!" << std::endl;
			std::cerr << "Required opengl version:" << std::endl;
			std::cerr << "  OpenGL version 2.0 or higher" << std::endl;
			std::cerr << "Required opengl extensions:" << std::endl;
			std::cerr << "  GL_ARB_texture_float" << std::endl;
			std::cerr << "  GL_ARB_point_sprite" << std::endl;
			std::cerr << "  GL_ARB_framebuffer_object" << std::endl;
			std::cerr << "Required constants:" << std::endl;
			std::cerr << "  GL_MAX_COLOR_ATTACHMENTS >= 4" << std::endl;
			fplConsoleWaitForCharInput();
			gladUnload();
			fplPlatformRelease();
			return(1);
		} else {
			fplConsoleFormatOut("ok\n");
		}

		fplConsoleFormatOut("Initialize Renderer\n");
		gRenderer = new CRenderer();

		fplConsoleFormatOut("Initialize Resources\n");
		InitResources(appPath.c_str());

		fplConsoleFormatOut("Load Fluid Scenarios\n");
		LoadFluidScenarios(appPath.c_str());

		fplConsoleFormatOut("Initialize PhysX\n");
		InitializePhysX();

		fplConsoleFormatOut("Load Fluid Scenario\n");
		ResetScene(*gScene);

		fplEvent ev;

		float frametime = 1.0f / 60.0f;
		fplWallClock lastTime = fplGetWallClock();
		fplConsoleFormatOut("Main loop\n\n");
		while(fplWindowUpdate()) {
			while(fplPollEvent(&ev)) {
				switch(ev.type) {
					case fplEventType_Keyboard:
						if(ev.keyboard.type == fplKeyboardEventType_Button) {
							if(ev.keyboard.buttonState == fplButtonState_Release) {
								KeyUp(ev.keyboard.mappedKey, 0, 0);
							} else {
								KeyDown(ev.keyboard.mappedKey, 0, 0);
							}
						}
						break;

					case fplEventType_Window:
						if(ev.window.type == fplWindowEventType_Resized) {
						}
						break;

					case fplEventType_Mouse:
						if(ev.mouse.type == fplMouseEventType_Move) {
							OnMouseMove(ev.mouse.mouseButton, ev.mouse.buttonState, ev.mouse.mouseX, ev.mouse.mouseY);
						} else if(ev.mouse.type == fplMouseEventType_Button) {
							OnMouseButton(ev.mouse.mouseButton, ev.mouse.buttonState, ev.mouse.mouseX, ev.mouse.mouseY);
						}
						break;

				}
			}

			fplWindowSize winSize;
			fplGetWindowSize(&winSize);

			OnRender(winSize.width, winSize.height, frametime);

			fplWallClock endTime = fplGetWallClock();
			double wallDelta = fplGetWallDelta(lastTime, endTime);
			lastTime = endTime;
		}

		OnShutdown();

		gladUnload();
		fplPlatformRelease();

		return(0);
	} else {
		return(1);
	}
}

