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
#include "Primitives.h"
#include "TextureManager.h"
#include "FluidProperties.h"
#include "GeometryVBO.h"
#include "PhysicsEngine.h"

// Font
#include "TextureFont.h"
#include "Fonts.h"

#include "AllShaders.hpp"
#include "AllFBOs.hpp"
#include "AllActors.hpp"

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
// Physics
//
constexpr float PhysXInitDT = 0.000001f;
constexpr float PhysXUpdateDT = 1.0f / 60.0f;
static PhysicsEngine *gPhysics = nullptr;
static PhysicsParticleSystem *gPhysicsParticles = nullptr;
static bool gPhysicsUseGPUAcceleration = false;

// Window vars
constexpr int DefaultWindowWidth = 1280;
constexpr int DefaultWindowHeight = 720;
constexpr float DefaultFov = 60.0;
constexpr float DefaultZNear = 0.1f;
constexpr float DefaultZFar = 1000.0f;

// Actors
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
static const char *GetActorCreationKindName(const ActorCreationKind kind) {
	switch(kind) {
		case ActorCreationKind::RigidBox:
			return "Rigid / Box";
		case ActorCreationKind::RigidSphere:
			return "Rigid / Sphere";
		case ActorCreationKind::RigidCapsule:
			return "Rigid / Capsule";
		case ActorCreationKind::FluidDrop:
			return "Fluid / Drop";
		case ActorCreationKind::FluidPlane:
			return "Fluid / Plane";
		case ActorCreationKind::FluidCube:
			return "Fluid / Box";
		case ActorCreationKind::FluidSphere:
			return "Fluid / Sphere";
		default:
			return "Unknown";

	}
}
static ActorCreationKind gCurrentActorCreationKind = ActorCreationKind::FluidCube;

static std::vector<Actor *> gActors;

static bool gDrawWireframe = false;
static bool gDrawBoundBox = false;
static bool gHideStaticRigidBodies = false;
static bool gHideDynamicRigidBodies = false;
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

// Fluid
constexpr int MaxFluidParticleCount = 512000;

static CSphericalPointSprites *gPointSprites = nullptr;
static CPointSpritesShader *gPointSpritesShader = nullptr;

static FluidDebugType gFluidDebugType = FluidDebugType::Final;

// Fluid properties
struct SimProperties {
	FluidSimulationProperties sim;
	FluidRenderProperties render;
};
static SimProperties gCurrentProperties = {};

// Fluid modification
static int64_t gFluidLatestExternalAccelerationTime = -1;

// Fluid property realtime change
enum class FluidProperty: int {
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

inline bool PointInSphere(const glm::vec3 &spherePos, const float &sphereRadius, const glm::vec3 &point, const float particleRadius) {
	glm::vec3 distance = spherePos - point;
	float length = glm::length(distance);
	float sumRadius = sphereRadius + particleRadius;
	return length <= sumRadius;
}

static void AddFluid(PhysicsParticleSystem &particleSys, const FluidActor &container, const FluidType type) {
	uint32_t numParticles = 0;

	float distance = gCurrentProperties.sim.restParticleDistance;

	glm::vec3 vel = container.velocity;

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

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> velocities;
	if(type == FluidType::Drop) {
		// Single drop
		numParticles++;
		positions.push_back(glm::vec3(centerX, centerY, centerZ));
		velocities.push_back(vel);
	} else if(type == FluidType::Plane) {
		// Water plane
		float zpos = centerZ - (dZ / 2.0f);
		idx = 0;

		for(long z = 0; z < numZ; z++) {
			float xpos = centerX - (dX / 2.0f);

			for(long x = 0; x < numX; x++) {
				numParticles++;
				positions.push_back(glm::vec3(xpos, centerY, zpos));
				velocities.push_back(vel);
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
					positions.push_back(glm::vec3(xpos, ypos, zpos));
					velocities.push_back(vel);
					idx++;
					xpos += distance;
				}

				ypos += distance;
			}

			zpos += distance;
		}
	} else if(type == FluidType::Sphere) {
		// Water sphere
		glm::vec3 center = glm::vec3(centerX, centerY, centerZ);

		float zpos = centerZ - (dZ / 2.0f);
		idx = 0;

		for(long z = 0; z < numZ; z++) {
			float ypos = centerY - (dY / 2.0f);

			for(long y = 0; y < numY; y++) {
				float xpos = centerX - (dX / 2.0f);

				for(long x = 0; x < numX; x++) {
					glm::vec3 point = glm::vec3(xpos, ypos, zpos);

					if(PointInSphere(center, radius, point, gCurrentProperties.sim.particleRadius)) {
						numParticles++;
						positions.push_back(point);
						velocities.push_back(vel);
						idx++;
					}

					xpos += distance;
				}

				ypos += distance;
			}

			zpos += distance;
		}
	}

	PhysicsParticlesStorage storage = {};
	storage.positions = positions.data();
	storage.velocities = velocities.data();
	storage.numParticles = numParticles;

	if(gPhysics->AddParticles(&particleSys, storage)) {
		gTotalFluidParticles += numParticles;
	}
}

static void AddFluids(PhysicsParticleSystem &particleSys, const FluidType type) {
	for(size_t i = 0, count = gActors.size(); i < count; i++) {
		Actor *actor = gActors[i];
		if(actor->type == ActorType::Fluid) {
			FluidActor *fluidActor = static_cast<FluidActor *>(actor);
			if(fluidActor->time <= 0) {
				AddFluid(particleSys, *fluidActor, type);
			}
		}
	}
}

static PhysicsParticleSystem *CreateParticleFluidSystem(PhysicsEngine &engine) {
	FluidSimulationProperties particleSystemDesc = gCurrentProperties.sim;
	particleSystemDesc.Validate();
	PhysicsParticleSystem *result = engine.AddParticleSystem(particleSystemDesc, MaxFluidParticleCount);
	return(result);
}

static PhysicsRigidBody::MotionKind ToMotionKind(const ActorMovementType movementType) {
	switch(movementType) {
		case ActorMovementType::Dynamic:
			return PhysicsRigidBody::MotionKind::Dynamic;
		default:
			return PhysicsRigidBody::MotionKind::Static;
	}
}

static void AddBox(PhysicsEngine &physics, CubeActor &cube) {
	PhysicsShape shape = PhysicsShape::MakeBox(cube.halfExtents);
	PhysicsRigidBody *rigidBody = physics.AddRigidBody(ToMotionKind(cube.movementType), cube.transform.position, cube.transform.rotation, shape);
	cube.physicsData = rigidBody;
}

static void AddSphere(PhysicsEngine &physics, SphereActor &sphere) {
	PhysicsShape shape = PhysicsShape::MakeSphere(sphere.radius);
	PhysicsRigidBody *rigidBody = physics.AddRigidBody(ToMotionKind(sphere.movementType), sphere.transform.position, sphere.transform.rotation, shape);
	sphere.physicsData = rigidBody;
}

static void AddCapsule(PhysicsEngine &physics, CapsuleActor &capsule) {
	PhysicsShape shape = PhysicsShape::MakeCapsule(capsule.radius, capsule.halfHeight);
	PhysicsRigidBody *rigidBody = physics.AddRigidBody(ToMotionKind(capsule.movementType), capsule.transform.position, capsule.transform.rotation, shape);
	capsule.physicsData = rigidBody;
}

static void AddPlane(PhysicsEngine &physics, PlaneActor &plane) {
	PhysicsShape shape = PhysicsShape::MakePlane();
	PhysicsRigidBody *rigidBody = physics.AddRigidBody(ToMotionKind(plane.movementType), plane.transform.position, plane.transform.rotation, shape);
	plane.physicsData = rigidBody;
}

static void AddScenarioActor(PhysicsEngine &physics, Actor *actor) {
	PhysicsShape shape = {};
	if(actor->type == ActorType::Cube) {
		CubeActor *cube = static_cast<CubeActor *>(actor);
		AddBox(physics, *cube);
	} else if(actor->type == ActorType::Sphere) {
		SphereActor *sphere = static_cast<SphereActor *>(actor);
		AddSphere(physics, *sphere);
	} else if(actor->type == ActorType::Capsule) {
		CapsuleActor *capsule = static_cast<CapsuleActor *>(actor);
		AddCapsule(physics, *capsule);
	} else if(actor->type == ActorType::Plane) {
		PlaneActor *plane = static_cast<PlaneActor *>(actor);
		AddPlane(physics, *plane);
	} else {
		assert(!"Actor type not supported");
	}
}

static void SaveFluidPositions(PhysicsParticleSystem &particleSys) {
	float *data = gPointSprites->Map();
	bool noDensity = gSSFRenderMode == SSFRenderMode::Points;
	particleSys.WriteToPositionBuffer(data, MaxFluidParticleCount, noDensity, gCurrentProperties.render.minDensity);
	gPointSprites->UnMap();
}

static void SingleStepPhysX(const float frametime) {
	// Advance simulation
	gPhysics->Simulate(frametime);

	// Save fluid positions
	if(gSSFRenderMode != SSFRenderMode::Disabled)
		SaveFluidPositions(*gPhysicsParticles);
}

static void ClearScene(PhysicsEngine &physics) {
	// Destroy all physics actors
	physics.Clear();
	gPhysicsParticles = nullptr;
	gTotalFluidParticles = 0;

	// Delete all actors
	for(size_t index = 0, count = gActors.size(); index < count; ++index) {
		Actor *actor = gActors[index];
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

static glm::quat RotateQuat(const float radians, const glm::vec3 &axis) {
	glm::vec3 axisNorm = glm::normalize(axis);
	float w = glm::cos(radians / 2);
	float v = glm::sin(radians / 2);
	glm::vec3 qv = axisNorm * v;
	glm::quat result(w, qv);
	return(result);
}

static void ResetScene(PhysicsEngine &physics) {
	assert(gActiveFluidScenario != nullptr);

	printf("Load/Reload scene: %s\n", gActiveFluidScenario->displayName);

	ClearScene(physics);

	// Set scene properties
	physics.SetGravity(gActiveFluidScenario->gravity);

	// Set particle properties
	gCurrentProperties.sim = gActiveFluidScenario->sim;
	gCurrentProperties.render = gActiveFluidScenario->render;
	gRigidBodyFallPos = gActiveFluidScenario->actorCreatePosition;

	// Add ground plane
	PlaneActor *groundPlane = new PlaneActor();
	groundPlane->transform.position = glm::vec3(0, 0, 0);
	groundPlane->transform.rotation = RotateQuat((float)M_PI * 0.5f, glm::vec3(0, 0, 1));
	AddPlane(physics, *groundPlane);
	gActors.push_back(groundPlane);

	// Create fluid system
	gPhysicsParticles = CreateParticleFluidSystem(physics);
	assert(gPhysicsParticles != nullptr);
	ParticleSystemActor *mainFluid = new ParticleSystemActor();
	mainFluid->physicsData = gPhysicsParticles;
	gActors.push_back(mainFluid);

	// Set GPU acceleration for particle fluid if supported
	physics.SetGPUAcceleration(gPhysicsUseGPUAcceleration);

	// Add bodies immediately from scenario
	for(size_t i = 0, count = gActiveFluidScenario->bodies.size(); i < count; i++) {
		const Actor *sourceActor = gActiveFluidScenario->bodies[i];
		Actor *targetActor = CloneBodyActor(sourceActor);
		gActors.push_back(targetActor);
		if(targetActor != nullptr) {
			targetActor->timeElapsed = 0.0f;
			if(targetActor->time == -1) {
				AddScenarioActor(physics, targetActor);
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
			AddFluid(*gPhysicsParticles, *targetActor, targetActor->fluidType);
		}
	}

	gTotalTimeElapsed = 0;
	gPhysicsAccumulator = 0;
	SingleStepPhysX(PhysXInitDT);
}

void InitializePhysics() {
	// CPU Dispatcher based on number of cpu cores
	uint32_t coreCount = COSLowLevel::getNumCPUCores();
	uint32_t numThreads = std::min(gActiveScene->numCPUThreads, coreCount);
	printf("  CPU core count: %lu\n", coreCount);
	printf("  CPU acceleration supported (%d threads)\n", numThreads);

	PhysicsEngineConfiguration config = PhysicsEngineConfiguration();
	config.threadCount = numThreads;

	gPhysics = PhysicsEngine::Create(config);

	gPhysicsUseGPUAcceleration = gPhysics->IsGPUAcceleration();
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
			gPhysicsParticles->SetExternalAcceleration(glm::vec3(0.0f, 0.0f, 0.0f));
			gFluidLatestExternalAccelerationTime = -1;
		}
	}

	// Update PhysX
	if(!gPaused) {
		SingleStepPhysX(frametime);
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

glm::mat4 ComputeGlobalPose(const PhysicsTransform &bodyTransform, const PhysicsTransform &shapeTransform) {
	glm::mat4 world = glm::translate(glm::mat4(1.0f), bodyTransform.pos) * glm::mat4(bodyTransform.rotation);
	glm::mat4 local = glm::translate(glm::mat4(1.0f), shapeTransform.pos) * glm::mat4(shapeTransform.rotation);
	glm::mat4 result = world * local;
	return(result);
}

void DrawBoxShape(const PhysicsTransform &bodyTransform, const PhysicsTransform &shapeTransform, const PhysicsBoxShape &box, const glm::vec4 &color) {
	glm::vec3 halfExtents = box.halfExtents;
	glm::mat4 mat = ComputeGlobalPose(bodyTransform, shapeTransform);
	glm::mat4 scaled = glm::scale(mat, glm::vec3(halfExtents.x, halfExtents.y, halfExtents.z));
	glm::mat4 mvp = gCamera.mvp * scaled;
	gRenderer->LoadMatrix(mvp);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);
	DrawPrimitive(gBoxVBO, false);
	gLightingShader->disable();
}

void DrawSphereShape(const PhysicsTransform &bodyTransform, const PhysicsTransform &shapeTransform, const PhysicsSphereShape &sphere, const glm::vec4 &color) {
	float radius = sphere.radius;

	glm::mat4 mat = ComputeGlobalPose(bodyTransform, shapeTransform);
	glm::mat4 scaled = glm::scale(mat, glm::vec3(radius));
	glm::mat4 mvp = gCamera.mvp * scaled;
	gRenderer->LoadMatrix(mvp);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);
	DrawPrimitive(gSphereVBO, false);
	gLightingShader->disable();
}

void DrawCapsuleShape(const PhysicsTransform &bodyTransform, const PhysicsTransform &shapeTransform, const PhysicsCapsuleShape &capsule, const glm::vec4 &color) {
	float radius = capsule.radius;
	float halfHeight = capsule.halfHeight;

	glm::mat4 mat = ComputeGlobalPose(bodyTransform, shapeTransform);
	glm::mat4 multm = gCamera.mvp * mat;

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);

	glm::mat4 rotation = glm::rotate(multm, Deg2Rad(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 translation0 = glm::translate(rotation, glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 scaled0 = glm::scale(translation0, glm::vec3(radius, radius, 2.0f * halfHeight));
	gRenderer->LoadMatrix(scaled0);
	DrawPrimitive(gCylinderVBO, false);

	glm::mat4 translation1 = glm::translate(rotation, glm::vec3(0.0f, 0.0f, -halfHeight));
	glm::mat4 scaled1 = glm::scale(translation1, glm::vec3(radius));
	gRenderer->LoadMatrix(scaled1);
	DrawPrimitive(gSphereVBO, false);

	glm::mat4 translation2 = glm::translate(rotation, glm::vec3(0.0f, 0.0f, halfHeight));
	glm::mat4 scaled2 = glm::scale(translation2, glm::vec3(radius));
	gRenderer->LoadMatrix(scaled2);
	DrawPrimitive(gSphereVBO, false);

	gLightingShader->disable();
}

void DrawShape(const PhysicsTransform &bodyTransform, const PhysicsShape &shape, const glm::vec4 &color) {
	switch(shape.type) {
		case PhysicsShape::Type::Box:
			DrawBoxShape(bodyTransform, shape.local, shape.box, color);
			break;

		case PhysicsShape::Type::Sphere:
			DrawSphereShape(bodyTransform, shape.local, shape.sphere, color);
			break;

		case PhysicsShape::Type::Capsule:
			DrawCapsuleShape(bodyTransform, shape.local, shape.capsule, color);
			break;
	}
}

void DrawBounds(const PhysicsBoundingBox &bounds) {
	glm::vec3 center = bounds.GetCenter();
	glm::vec3 ext = bounds.GetSize() * 0.5f;

	GLfloat mat_diffuse[4] = { 0, 1, 1, 1 };
	glColor4fv(mat_diffuse);

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), center);
	glm::mat4 scaled = glm::scale(translation, ext);
	glm::mat4 mvp = gCamera.mvp * scaled;
	gRenderer->LoadMatrix(mvp);
	DrawPrimitive(gBoxVBO, true);
}

void DrawActorBounds(const PhysicsActor &physicsActor) {
	PhysicsBoundingBox bounds = physicsActor.bounds;
	if(gFrustum.containsBounds(bounds.min, bounds.max)) {
		gDrawedActors++;
		DrawBounds(bounds);
	}
}

void DrawRigidBody(const Actor &actor, const PhysicsRigidBody &rigidBody, const bool isVisible, const bool isBlending) {
	PhysicsBoundingBox bounds = rigidBody.bounds;
	if(gFrustum.containsBounds(bounds.min, bounds.max)) {
		if(isVisible) {

			if(rigidBody.motionKind == PhysicsRigidBody::MotionKind::Dynamic && !gHideDynamicRigidBodies ||
				rigidBody.motionKind == PhysicsRigidBody::MotionKind::Static && !gHideStaticRigidBodies) {

				if(isBlending) {
					gRenderer->SetBlending(true);
					glDisable(GL_DEPTH_TEST);
				}

				for(size_t shapeIndex = 0; shapeIndex < rigidBody.shapeCount; ++shapeIndex) {
					const PhysicsShape &shape = rigidBody.shapes[shapeIndex];
					DrawShape(rigidBody.transform, shape, actor.color);
				}

				if(isBlending) {
					gRenderer->SetBlending(false);
					glEnable(GL_DEPTH_TEST);
				}

				gDrawedActors++;
			}
		}
	}
}

void RenderActors() {
	// Render all the actors in the scene
	for(size_t index = 0, count = gActors.size(); index < count; ++index) {
		Actor *actor = gActors[index];
		if(actor->physicsData != nullptr) {
			PhysicsActor *pactor = static_cast<PhysicsActor *>(actor->physicsData);
			if(pactor->type == PhysicsActor::Type::RigidBody) {
				PhysicsRigidBody *rigidBody = static_cast<PhysicsRigidBody *>(pactor);
				DrawRigidBody(*actor, *rigidBody, actor->visible, actor->blending);
			}
		}
	}
}

void RenderActorBoundings() {
	// Render all the actors in the scene as bounding volume
	for(size_t index = 0, count = gActors.size(); index < count; ++index) {
		Actor *actor = gActors[index];
		if(actor->physicsData != nullptr) {
			PhysicsActor *pactor = static_cast<PhysicsActor *>(actor->physicsData);
			DrawActorBounds(*pactor);
		}
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

		case FluidProperty::StaticFriction:
			return "Static friction\0";

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
	assert(gPhysics != nullptr);
	assert(gPhysicsParticles != nullptr);

	// Add not fallen fluids from active scenario

	// Add actors
	for(size_t i = 0, count = gActors.size(); i < count; i++) {
		Actor *actor = gActors[i];
		if(actor->type != ActorType::Fluid) {
			if(actor->time > 0) {
				if(actor->timeElapsed < (float)actor->time) {
					actor->timeElapsed += frametime;
					if(actor->timeElapsed >= (float)actor->time) {
						AddScenarioActor(*gPhysics, actor);
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
						AddFluid(*gPhysicsParticles, *fluid, fluid->fluidType);
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
							AddFluid(*gPhysicsParticles, *fluid, fluid->fluidType);
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
		sprintf_s(buffer, "Geometry type (1-7): %s", GetActorCreationKindName(gCurrentActorCreationKind));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Wireframe (W): %s", gDrawWireframe ? "enabled" : "disabled");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Boundbox (B): %s", gDrawBoundBox ? "enabled" : "disabled");
		RenderOSDLine(osdPos, buffer);
#if 0
		sprintf_s(buffer, "Draw Rigidbodies (D): %s", GetDrawRigidbodyStr(gHideRigidBodies));
		RenderOSDLine(osdPos, buffer);
#endif
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
		sprintf_s(buffer, "    Fluid viscosity: %f", gCurrentProperties.sim.viscosity);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid stiffness: %f", gCurrentProperties.sim.stiffness);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid rest offset: %f", gCurrentProperties.sim.restOffset);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid contact offset: %f", gCurrentProperties.sim.contactOffset);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid restitution: %f", gCurrentProperties.sim.restitution);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid damping: %f", gCurrentProperties.sim.damping);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid dynamic friction: %f", gCurrentProperties.sim.dynamicFriction);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid static friction: %f", gCurrentProperties.sim.staticFriction);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid particle mass: %f", gCurrentProperties.sim.particleMass);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid max motion distance: %f", gCurrentProperties.sim.maxMotionDistance);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid blur depth scale: %f", gSSFBlurDepthScale);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid particle render factor: %f", gCurrentProperties.render.particleRenderFactor);
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
		sprintf_s(buffer, "Fluid using GPU acceleration (H): %s", gPhysicsUseGPUAcceleration ? "yes" : "no");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid emitter active (K): %s", !gStoppedEmitter ? "yes" : "no");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Add fluid after scene change (N): %s", !gWaterAddBySceneChange ? "yes" : "no");
		RenderOSDLine(osdPos, buffer);

		// Empty line
		osdPos.newLine();

		sprintf_s(buffer, "Fluid particle radius: %f", gCurrentProperties.sim.particleRadius);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid rest particle distance: %f", gCurrentProperties.sim.restParticleDistance);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid cell size: %f", gCurrentProperties.sim.cellSize);
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
		gFluidRenderer->Render(gCamera, gTotalFluidParticles, options, windowWidth, windowHeight, gCurrentProperties.sim.particleRadius * gCurrentProperties.render.particleRenderFactor);
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

static void AddDynamicActor(PhysicsEngine &physics, PhysicsParticleSystem &particleSys, const ActorCreationKind kind) {
	glm::vec3 pos = gRigidBodyFallPos;
	glm::vec3 vel = DefaultRigidBodyVelocity;
	float density = DefaultRigidBodyDensity;

	// TODO(final): Random Y rotation
	glm::quat rotation = glm::quat(glm::vec3(0));

	//glm::quat rotation = toGLMQuat(physx::PxQuat(Deg2Rad(RandomRadius()), physx::PxVec3(0, 1, 0)));

	switch(kind) {
		case ActorCreationKind::RigidBox:
		{
			CubeActor *box = new CubeActor(ActorMovementType::Dynamic, glm::vec3(0.5, 0.5, 0.5));
			box->color = glm::vec4(0.0f, 0.1f, 1.0f, 1.0f);
			box->transform.position = pos;
			box->transform.rotation = rotation;
			box->velocity = vel;
			box->density = density;
			AddBox(physics, *box);
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
			AddSphere(physics, *sphere);
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
			AddCapsule(physics, *capsule);
			gActors.push_back(capsule);
		} break;

		case ActorCreationKind::FluidDrop:
			AddFluids(particleSys, FluidType::Drop);
			break;

		case ActorCreationKind::FluidPlane:
			AddFluids(particleSys, FluidType::Plane);
			break;

		case ActorCreationKind::FluidCube:
			AddFluids(particleSys, FluidType::Box);
			break;

		case ActorCreationKind::FluidSphere:
			AddFluids(particleSys, FluidType::Sphere);
			break;

		default:
			break;
	}
}

void ToggleFluidGPUAcceleration() {
	bool enabled = gPhysics->IsGPUAcceleration();
	gPhysics->SetGPUAcceleration(!enabled);
}

void SetFluidExternalAcceleration(const glm::vec3 &acc) {
	gFluidLatestExternalAccelerationTime = fplGetTimeInMillisecondsLP() + 3000; // 3 Seconds
	gPhysicsParticles->SetExternalAcceleration(acc);
}

void KeyUp(const fplKey key, const int x, const int y) {
	assert(gPhysics != nullptr);
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
			ResetScene(*gPhysics);
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
#if 0
			gHideRigidBodies++;

			if(gHideRigidBodies > HideRigidBody_MAX)
				gHideRigidBodies = 0;
#endif

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
				ResetScene(*gPhysics);
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

			SingleStepPhysX(PhysXInitDT);

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
			gCurrentProperties.sim.viscosity += value;
			gPhysicsParticles->SetViscosity(gCurrentProperties.sim.viscosity);
			gActiveFluidScenario->sim.viscosity = gCurrentProperties.sim.viscosity;
		} break;

		case FluidProperty::Stiffness:
		{
			gCurrentProperties.sim.stiffness += value;
			gPhysicsParticles->SetStiffness(gCurrentProperties.sim.stiffness);
			gActiveFluidScenario->sim.stiffness = gCurrentProperties.sim.stiffness;
		} break;

		case FluidProperty::MaxMotionDistance:
		{
			gCurrentProperties.sim.maxMotionDistance += value / 1000.0f;
			gPhysicsParticles->SetMaxMotionDistance(gCurrentProperties.sim.maxMotionDistance);
			gActiveScene->sim.maxMotionDistance = gCurrentProperties.sim.maxMotionDistance;
		} break;

		case FluidProperty::ContactOffset:
		{
			gCurrentProperties.sim.contactOffset += value / 1000.0f;
			gPhysicsParticles->SetContactOffset(gCurrentProperties.sim.contactOffset);
			gActiveScene->sim.contactOffset = gCurrentProperties.sim.contactOffset;
		} break;

		case FluidProperty::RestOffset:
		{
			gCurrentProperties.sim.restOffset += value / 1000.0f;
			gPhysicsParticles->SetRestOffset(gCurrentProperties.sim.restOffset);
			gActiveScene->sim.restOffset = gCurrentProperties.sim.restOffset;
		} break;

		case FluidProperty::Restitution:
		{
			gCurrentProperties.sim.restitution += value / 1000.0f;
			gPhysicsParticles->SetRestitution(gCurrentProperties.sim.restitution);
			gActiveScene->sim.restitution = gCurrentProperties.sim.restitution;
		} break;

		case FluidProperty::Damping:
		{
			gCurrentProperties.sim.damping += value / 1000.0f;
			gPhysicsParticles->SetDamping(gCurrentProperties.sim.damping);
			gActiveScene->sim.damping = gCurrentProperties.sim.damping;
		} break;

		case FluidProperty::DynamicFriction:
		{
			gCurrentProperties.sim.dynamicFriction += value / 1000.0f;
			gPhysicsParticles->SetDynamicFriction(gCurrentProperties.sim.dynamicFriction);
			gActiveScene->sim.dynamicFriction = gCurrentProperties.sim.dynamicFriction;
		} break;

		case FluidProperty::StaticFriction:
		{
			gCurrentProperties.sim.staticFriction += value / 1000.0f;
			gPhysicsParticles->SetStaticFriction(gCurrentProperties.sim.staticFriction);
			gActiveScene->sim.staticFriction = gCurrentProperties.sim.staticFriction;
		} break;

		case FluidProperty::ParticleMass:
		{
			gCurrentProperties.sim.particleMass += value / 1000.0f;
			gPhysicsParticles->SetParticleMass(gCurrentProperties.sim.particleMass);
			gActiveScene->sim.particleMass = gCurrentProperties.sim.particleMass;
		} break;

		case FluidProperty::DepthBlurScale:
		{
			gSSFBlurDepthScale += value / 10000.0f;
		} break;

		case FluidProperty::ParticleRenderFactor:
		{
			gCurrentProperties.render.particleRenderFactor += value / 10.0f;
			gCurrentProperties.render.particleRenderFactor = roundFloat(gCurrentProperties.render.particleRenderFactor);
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

void KeyDown(const fplKey key, const int x, const int y) {
	const float accSpeed = 10.0f;
	const PhysicsForceMode accMode = PhysicsForceMode::Acceleration;

	switch(key) {
		case fplKey_Right:
		{
			gPhysicsParticles->AddForce(glm::vec3(1.0f, 0.0f, 0.0f) * accSpeed, accMode);
			break;
		}

		case fplKey_Left:
		{
			gPhysicsParticles->AddForce(glm::vec3(-1.0f, 0.0f, 0.0f) * accSpeed, accMode);
			break;
		}

		case fplKey_Up:
		{
			gPhysicsParticles->AddForce(glm::vec3(0.0f, 0.0f, -1.0f) * accSpeed, accMode);
			break;
		}

		case fplKey_Down:
		{
			gPhysicsParticles->AddForce(glm::vec3(0.0f, 0.0f, 1.0f) * accSpeed, accMode);
			break;
		}

		case fplKey_Space: // Space
		{
			AddDynamicActor(*gPhysics, *gPhysicsParticles, gCurrentActorCreationKind);
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
	gActiveScene = new CScene(DefaultRigidBodyDensity);
	gActiveScene->load("scene.xml");
	gCurrentProperties.sim = gActiveScene->sim;
	gCurrentProperties.render = gActiveScene->render;
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
	// Initial FBO size does not matter, because its resized on render anyway
	gFluidRenderer = new CScreenSpaceFluidRendering(128, 128);
	gFluidRenderer->SetRenderer(gRenderer);
	gFluidRenderer->SetPointSprites(gPointSprites);
	gFluidRenderer->SetPointSpritesShader(gPointSpritesShader);
	gFluidRenderer->SetSceneTexture(gSceneFBO->sceneTexture);
	gFluidRenderer->SetSkyboxCubemap(gSkyboxCubemap);

	// Create lightning shader
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
	printf("Release Physics\n");
	if(gPhysics != nullptr) {
		delete gPhysics;
		gPhysics = nullptr;
	}

	printf("Release Resources\n");
	ReleaseResources();

	// Release scenarios
	printf("Release Fluid Scenarios\n");
	for(unsigned int i = 0; i < gFluidScenarios.size(); i++) {
		Scenario *scenario = gFluidScenarios[i];
		delete scenario;
	}
	gFluidScenarios.clear();

	// Release renderer
	printf("Release Renderer\n");
	if(gRenderer) {
		delete gRenderer;
		gRenderer = nullptr;
	}
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
		InitializePhysics();

		fplConsoleFormatOut("Load Fluid Scenario\n");
		ResetScene(*gPhysics);

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

