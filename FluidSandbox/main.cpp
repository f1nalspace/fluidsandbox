/*
=======================================================================================
	Fluid Sandbox

	A realtime application for playing around with 3D fluids and rigid bodies.
	Features
	- Fluid and rigid body simulation based on NVidia PhysX
	- Screen Space Fluid Rendering
	- Customizable Scenes/Scenarios
	- Custom OpenGL Rendering Engine

	This is the main source file.

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	See license below.
=======================================================================================
Dependencies:

	- Visual Studio 2019
	- PhysX SDK 3.4.2 (Multithreaded DLL, x64/win32)
	- FreeGlut (Multithreaded DLL, x64/win32)
	- FreeImage (Multithreaded DLL, x64/win32)
	- Glew (Multithreaded DLL, x64/win32)
	- glm
	- rapidxml
=======================================================================================
Todo:

	- Replace freeglut with final_platform_layer.h
	- Replace FreeImage with stb_image.h
	- Replace Glew with final_dynamic_opengl.h
	- Replace rapidxml with final_xml.h
	- Add proper OSD rendering using stb_freetype.h

	- No more pragma comment lib for the libraries, just configure it in the project directly

	- Update positions, rotations, etc. in source actor from PhysX actor

	- Use source actor for rendering instead of PhysX actor

	- Move all physics code into its own class, so we can swap physics engine any time

	- More cameras (Free, Rotate around point, Fixed)

	- More scenarios

	- Use unsigned integer or size_t instead of integer (Unsigned by default)

	- Migrate to OpenGL 3.x

	- Abstract rendering so we can support multiple renderer (GL 3.x, Vulkan)
=======================================================================================
License:

	This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0
	You can find a a copy of the license file in the source directory (LICENSE.txt)
	If a copy of the MPL was not distributed with this file, You can obtain one at https://www.mozilla.org/MPL/2.0/.

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
=======================================================================================
*/

// Enable this to activate support for PhysX Visual Debugger
//#define PVD_ENABLED

#include <iostream>
#include <vector>
#include <time.h>
#include <malloc.h>
#include <string.h>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <math.h>
#include <typeinfo>

// OpenGL (Glut + Glew)
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
// OpenGL required libs
#pragma comment(lib, "glew32.lib")

// OpenGL mathmatics
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>

// PhysX API
#include <PxPhysicsAPI.h>

// PhysX required libs
#ifdef _DEBUG
#   if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
#       pragma comment(lib, "PhysX3DEBUG_x64.lib")
#       pragma comment(lib, "PhysX3CommonDEBUG_x64.lib")
#       pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#       pragma comment(lib, "PxFoundationDEBUG_x64.lib")
#       pragma comment(lib, "PxTaskDEBUG_x64.lib")
#       pragma comment(lib, "PxPvdSDKDEBUG_x64.lib")
#   else
#       pragma comment(lib, "PhysX3DEBUG_x86.lib")
#       pragma comment(lib, "PhysX3CommonDEBUG_x86.lib")
#       pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#       pragma comment(lib, "PxFoundationDEBUG_x86.lib")
#       pragma comment(lib, "PxTaskDEBUG_x86.lib")
#       pragma comment(lib, "PxPvdSDKDEBUG_x86.lib")
#   endif
#else
#   if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
#       pragma comment(lib, "PhysX3_x64.lib")
#       pragma comment(lib, "PhysX3Common_x64.lib")
#       pragma comment(lib, "PhysX3Extensions.lib")
#       pragma comment(lib, "PxFoundation_x64.lib")
#       pragma comment(lib, "PxTask_x64.lib")
#       pragma comment(lib, "PxPvdSDK_x64.lib")
#   else
#       pragma comment(lib, "PhysX3_x86.lib")
#       pragma comment(lib, "PhysX3Common_x86.lib")
#       pragma comment(lib, "PhysX3Extensions.lib")
#       pragma comment(lib, "PxFoundation_x86.lib")
#       pragma comment(lib, "PxTask_x86.lib")
#       pragma comment(lib, "PxPvdSDK_x86.lib")
#       pragma comment(lib, "PhysX3_x86.lib")
#   endif
#endif

// XML
#include "rapidxml/rapidxml.hpp"

// Free image
#include "freeimage/FreeImage.h"
#pragma comment(lib, "FreeImage.lib")

// Classes
#include "Frustum.h"
#include "OSLowLevel.h"
#include "GLSL.h"
#include "SphericalPointSprites.h"
#include "Renderer.h"
#include "Camera.hpp"
#include "Utils.h"
#include "ScreenSpaceFluidRendering.h"
#include "FluidScenario.h"
#include "Actor.hpp"
#include "Scene.h"
#include "FluidSystem.h"
#include "Primitives.h"
#include "TextureManager.h"
#include "FluidProperties.h"
#include "Light.h"
#include "GLSLManager.h"

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

// App path
std::string appPath = "";

// Application
const char *APPLICATION_NAME = "Fluid Sandbox";
const char *APPLICATION_VERSION = "1.8.0";
const char *APPLICATION_AUTHOR = "Torsten Spaete";
const char *APPLICATION_COPYRIGHT = "© 2015-2021 Torsten Spaete - All rights reserved";
const std::string APPTITLE = APPLICATION_NAME + std::string(" v") + APPLICATION_VERSION + std::string(" by ") + APPLICATION_AUTHOR;

// Math stuff
const float DEG2RAD = (float)M_PI / 180.0f;

// PhysX Debugger
const char *PVD_Host = "localhost";
const int PVD_Port = 5425;

// PhysX SDK & Scene
static physx::PxFoundation *gPhysXFoundation = NULL;
static physx::PxPhysics *gPhysicsSDK = NULL;
static physx::PxDefaultErrorCallback gDefaultErrorCallback;
static physx::PxDefaultAllocator gDefaultAllocatorCallback;
static physx::PxSimulationFilterShader gDefaultFilterShader = physx::PxDefaultSimulationFilterShader;
static physx::PxMaterial *gDefaultMaterial = NULL;
static physx::PxScene *gScene = NULL;
#ifdef PVD_ENABLED
static physx::PxPvd *gPhysXVisualDebugger = NULL;
static physx::PxPvdTransport *gPhysXPvdTransport = NULL;
static bool gIsPhysXPvdConnected = false;
#endif

// For GPU Acceleration
static physx::PxGpuDispatcher *gGPUDispatcher = NULL;
static physx::PxCudaContextManager *gCudaContextManager = NULL;

// Window vars
static bool windowFullscreen = false;
static int windowWidth = 1280;
static int windowHeight = 720;
static int lastWindowWidth = windowWidth;
static int lastWindowHeight = windowHeight;
static int lastWindowPosX = 0;
static int lastWindowPosY = 0;

constexpr float defaultFov = 60.0;
constexpr float defaultZNear = 0.1f;
constexpr float defaultZFar = 1000.0f;
// PhysX Simulation step 60 steps per second
constexpr float gTimeSimulationStep = 1.0f / 60.0f;

static bool gStoppedEmitter = false;

// List of added actors and rigid bodies settings
static std::vector<physx::PxActor *> gActors;
constexpr float gDefaultRigidBodyDensity = 0.05f;
constexpr static glm::vec3 gDefaultRigidBodyVelocity(0.0f, 0.0f, 0.0f);

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

constexpr int HideRigidBody_None = 0;
constexpr int HideRigidBody_Blending = 1;
constexpr int HideRigidBody_All = 2;
constexpr int HideRigidBody_MAX = HideRigidBody_All;

// Various drawing states
static bool gDrawWireframe = false;
static bool gDrawBoundBox = false;
static int gHideRigidBodies = HideRigidBody_None;
static bool showOSD = false;

// Drawing statistics
static size_t gTotalActors = 0;
static size_t gDrawedActors = 0;
static uint32_t gTotalFluidParticles = 0;

// For simulation
constexpr int MAX_FLUID_PARTICLES = 512000;

static glm::vec3 gRigidBodyFallPos(0.0f, 10.0f, 0.0f);
static CFluidSystem *gFluidSystem = NULL;

// Debug types
static FluidDebugType gFluidDebugType = FluidDebugType::Final;

static CSphericalPointSprites *gPointSprites = NULL;
static CPointSpritesShader *gPointSpritesShader = NULL;
static CLightingShader *gLightingShader = NULL;

static bool gFluidUseGPUAcceleration = false;

// 45 - 60 nvidia, 80 - 40 is better for this, 20 - 35 is a good value for water
static float gFluidViscosity = FluidSimulationProperties::DefaultViscosity;
static float gFluidStiffness = FluidSimulationProperties::DefaultStiffness;
static float gFluidRestOffset = 0.0f;
static float gFluidContactOffset = 0.0f;
static float gFluidRestParticleDistance = 0.0f;
static float gFluidMaxMotionDistance = 0.0f;
static float gFluidRestitution = 0.0f;
static float gFluidDamping = 0.0f;
static float gFluidDynamicFriction = 0.0f;
static float gFluidParticleMass = 0.0f;
static float gFluidParticleRadius = 0.0f;
static float gFluidParticleRenderFactor = 0.0f;

static std::vector<FluidScenario *> gFluidScenarios;
static FluidScenario *gActiveFluidScenario = NULL;
static int gActiveFluidScenarioIdx = -1;

// Fluid modification
static int gFluidLatestExternalAccelerationTime = -1;

// Fluid property realtime change
const unsigned int FLUID_PROPERTY_NONE = 0;
const unsigned int FLUID_PROPERTY_VISCOSITY = 1;
const unsigned int FLUID_PROPERTY_STIFFNESS = 2;
const unsigned int FLUID_PROPERTY_MAXMOTIONDISTANCE = 3;
const unsigned int FLUID_PROPERTY_CONTACTOFFSET = 4;
const unsigned int FLUID_PROPERTY_RESTOFFSET = 5;
const unsigned int FLUID_PROPERTY_RESTITUTION = 6;
const unsigned int FLUID_PROPERTY_DAMPING = 7;
const unsigned int FLUID_PROPERTY_DYNAMICFRICTION = 8;
const unsigned int FLUID_PROPERTY_PARTICLEMASS = 9;
const unsigned int FLUID_PROPERTY_DEPTH_BLUR_SCALE = 10;
const unsigned int FLUID_PROPERTY_PARTICLE_RENDER_FACTOR = 11;
const unsigned int FLUID_PROPERTY_COLOR_FALLOFF_SCALE = 12;
const unsigned int FLUID_PROPERTY_COLOR_FALLOFF_ALPHA = 13;
const unsigned int FLUID_PROPERTY_DEBUGTYPE = 14;
const unsigned int MAX_FLUID_PROPERTY = FLUID_PROPERTY_DEBUGTYPE;
static unsigned int gFluidCurrentProperty = FLUID_PROPERTY_NONE;

// For mouse dragging
static int oldX = 0, oldY = 0;
static float rX = 15, rY = 0;
static float fps = 0;
//int startTime=0;
static int totalFrames = 0;
static int state = 1;
static float dist = 15;
static int lastFrameTime = 0;

// Renderer
static CRenderer *gRenderer = NULL;

// Fluid Renderer
static CScreenSpaceFluidRendering *gFluidRenderer = NULL;
static SSFRenderMode gSSFRenderMode = SSFRenderMode::Fluid;
static float gSSFDetailFactor = 1.0f;
static float gSSFBlurDepthScale = 0.0008f;
static bool gSSFBlurActive = true;
static bool gWaterAddBySceneChange = true;

// Managers
static CTextureManager *gTexMng = NULL;
static CGLSLManager *gShaderMng = NULL;

// Current fluid color index
static int gSSFCurrentFluidIndex = 0;

// Current scene
static CScene *gActiveScene = NULL;

// Current camera
static CCamera gCamera;

// Non fluid rendering
static CSceneFBO *gSceneFBO = NULL;
static CGLSL *gSceneShader = NULL;
static CVBO *gSkyboxVBO = NULL;
static CSkyboxShader *gSkyboxShader = NULL;
static CTexture *gSkyboxCubemap = NULL;
static CTexture *gTestTexture = NULL;

// Timing
static float gTotalTimeElapsed = 0;
static bool paused = false;

// Default colors
constexpr static glm::vec4 DefaultStaticRigidBodyColor(0.0f, 0.0f, 0.1f, 0.3f);
constexpr static glm::vec4 DefaultDynamicRigidBodyCubeColor(0.85f, 0.0f, 0.0f, 1.0f);
constexpr static glm::vec4 DefaultDynamicRigidBodySphereColor(0, 0.85f, 0.0f, 1.0f);
constexpr static glm::vec4 DefaultDynamicRigidBodyCapsuleColor(0.85f, 0.85f, 0.0f, 1.0f);

struct OSDRenderPosition {
	int x;
	int y;
	OSDRenderPosition() {
		x = 0;
		y = 0;
	}

	void newLine() {
		y += 14;
	}
};

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

physx::PxQuat createQuatRotation(const physx::PxVec3 &rotate) {
	physx::PxQuat rot = physx::PxQuat(rotate.x, physx::PxVec3(1.0f, 0.0f, 0.0f));
	rot *= physx::PxQuat(rotate.y, physx::PxVec3(0.0f, 1.0f, 0.0f));
	rot *= physx::PxQuat(rotate.z, physx::PxVec3(0.0f, 0.0f, 1.0f));
	return rot;
}

void setActorDrain(physx::PxActor *actor, const bool enable) {
	assert(actor != NULL);

	physx::PxType actorType = actor->getConcreteType();
	if(actorType == physx::PxConcreteType::eRIGID_STATIC || actorType == physx::PxConcreteType::eRIGID_DYNAMIC) {
		physx::PxRigidActor *rigActor = (physx::PxRigidActor *)actor;
		physx::PxU32 nShapes = rigActor->getNbShapes();
		physx::PxShape **shapes = new physx::PxShape * [nShapes];
		rigActor->getShapes(shapes, nShapes);

		while(nShapes--) {
			physx::PxShapeFlags flags = shapes[nShapes]->getFlags();
			if(enable)
				flags |= physx::PxShapeFlag::ePARTICLE_DRAIN;
			else
				flags &= ~physx::PxShapeFlag::ePARTICLE_DRAIN;
			shapes[nShapes]->setFlags(flags);
		}

		delete[] shapes;
	}
}

std::string vecToString(const physx::PxVec3 &p) {
	char str[255];
	sprintf_s(str, "%f, %f, %f", p.x, p.y, p.z);
	return str;
}

void AddBox(const glm::vec3 &pos, const glm::quat &rotation, const glm::vec3 &vel, const float density, const glm::vec3 &size, const bool isParticleDrain, const ActorMovementType movementType, CubeActor *sourceActor) {
	// NOTE(final): Actor can be null

	// Create static box
	physx::PxVec3 npos = toPxVec3(pos);
	physx::PxVec3 nvel = toPxVec3(vel);
	physx::PxQuat nrot = toPxQuat(rotation);
	physx::PxTransform transform(npos, nrot);
	physx::PxVec3 nsize = toPxVec3(size);

	physx::PxBoxGeometry geometry(nsize);
	physx::PxActor *actor;
	if(movementType == ActorMovementType::Static) {
		actor = PxCreateStatic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial);
		assert(actor != nullptr);
	} else {
		physx::PxRigidDynamic *rigidbody = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial, density);
		assert(rigidbody != nullptr);
		actor = rigidbody;
		physx::PxRigidBodyExt::updateMassAndInertia(*rigidbody, density);
		rigidbody->setAngularDamping(0.75);
		rigidbody->setLinearVelocity(nvel);
	}

	actor->userData = sourceActor;

	gScene->addActor(*actor);
	gActors.push_back(actor);

	setActorDrain(actor, isParticleDrain);
}

void AddSphere(const glm::vec3 &pos, const glm::quat &rotation, const glm::vec3 &vel, const float density, const float radius, const ActorMovementType movementType, SphereActor *sphereActor) {
	// NOTE(final): Actor can be null

	// Create static sphere
	physx::PxVec3 npos = toPxVec3(pos);
	physx::PxVec3 nvel = toPxVec3(vel);
	physx::PxQuat nrot = toPxQuat(rotation);
	physx::PxTransform transform(npos, nrot);

	physx::PxSphereGeometry geometry(radius);
	physx::PxActor *actor;
	if(movementType == ActorMovementType::Static) {
		actor = PxCreateStatic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial);
		assert(actor != nullptr);
	} else {
		physx::PxRigidDynamic *rigidbody = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial, density);
		assert(rigidbody != nullptr);
		actor = rigidbody;
		physx::PxRigidBodyExt::updateMassAndInertia(*rigidbody, density);
		rigidbody->setAngularDamping(0.75);
		rigidbody->setLinearVelocity(nvel);
	}

	actor->userData = sphereActor;

	gScene->addActor(*actor);
	gActors.push_back(actor);
}

void AddCapsule(const glm::vec3 &pos, const glm::quat &rotation, const glm::vec3 &vel, const float density, const glm::vec2 ext, const ActorMovementType movementType) {
	// TODO(final): Capsule Actor!

	// Create capsule
	physx::PxVec3 npos = toPxVec3(pos);
	physx::PxVec3 nvel = toPxVec3(vel);
	physx::PxQuat nrot = toPxQuat(rotation);
	physx::PxTransform transform(npos, nrot);

	physx::PxCapsuleGeometry geometry(ext.x, ext.y);

	physx::PxActor *actor;
	if(movementType == ActorMovementType::Static) {
		actor = PxCreateStatic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial);
		assert(actor != nullptr);
	} else {
		physx::PxRigidDynamic *rigidbody = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial, density);
		assert(rigidbody != nullptr);
		actor = rigidbody;
		physx::PxRigidBodyExt::updateMassAndInertia(*rigidbody, density);
		rigidbody->setAngularDamping(0.75);
		rigidbody->setLinearVelocity(nvel);
	}

	gScene->addActor(*actor);

	gActors.push_back(actor);
}

bool PointInSphere(const physx::PxVec3 &spherePos, const float &sphereRadius, const physx::PxVec3 point, const float particleRadius) {
	physx::PxVec3 distance = spherePos - point;
	float length = distance.magnitude();
	float sumRadius = sphereRadius + particleRadius;
	return length <= sumRadius;
}

void AddWater(FluidContainer &container, const FluidType type) {
	uint32_t numParticles = 0;
	std::vector<physx::PxVec3> particlePositionBuffer;
	std::vector<physx::PxVec3> particleVelocityBuffer;

	float distance = gFluidRestParticleDistance;

	physx::PxVec3 vel = toPxVec3(container.vel);

	float centerX = container.pos.x;
	float centerY = container.pos.y;
	float centerZ = container.pos.z;

	float sizeX = container.size.x;
	float sizeY = container.size.y;
	float sizeZ = container.size.z;

	if(type == FluidType::Sphere) {
		if(container.radius < 0.00001f)
			container.radius = ((sizeX + sizeY + sizeZ) / 3.0f) / 2.0f;
	}

	long numX = (long)(container.size.x / distance);
	long numY = (long)(container.size.y / distance);
	long numZ = (long)(container.size.z / distance);

	float dX = distance * numX;
	float dY = distance * numY;
	float dZ = distance * numZ;

	int idx;

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
	} else if(type == FluidType::Sphere)  // Water sphere
	{
		physx::PxVec3 center = physx::PxVec3(centerX, centerY, centerZ);

		float radius = container.radius;
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
		gFluidSystem->createParticles(numParticles, &particlePositionBuffer[0], &particleVelocityBuffer[0]);
	}
}

void AddWater(FluidType waterType) {
	if(!gActiveFluidScenario) return;

	for(size_t i = 0; i < gActiveFluidScenario->getFluidContainerCount(); i++) {
		FluidContainer *container = gActiveFluidScenario->getFluidContainer(i);

		if(container->time <= 0)
			AddWater(*container, waterType);
	}
}

void AddPlane() {
	// Create ground plane
	physx::PxTransform pose = physx::PxTransform(physx::PxVec3(0.0f, 0, 0.0f), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));

	physx::PxRigidStatic *plane = gPhysicsSDK->createRigidStatic(pose);

	if(!plane)
		std::cerr << "create plane failed!" << std::endl;

	physx::PxShape *shape = plane->createShape(physx::PxPlaneGeometry(), *gDefaultMaterial);

	if(!shape)
		std::cerr << "create shape failed!" << std::endl;

	gScene->addActor(*plane);
	gActors.push_back(plane);
}

void ClearScene() {
	// Remove fluid system
	if(gFluidSystem)
		delete gFluidSystem;

	// Remove actors
	for(size_t index = 0; index < gActors.size(); ++index) {
		physx::PxActor *act = gActors[index];
		gScene->removeActor(*act);
		act->release();
	}

	gActors.clear();

	// Reset drawing statistics
	//gTotalFluidParticles = 0;
}

void AddScenarioActor(Actor *actor) {
	if(actor->type == ActorType::Cube) {
		CubeActor *cube = (CubeActor *)actor;
		AddBox(cube->pos, cube->rotation, cube->velocity, cube->density, cube->size, cube->particleDrain, cube->movementType, cube);
	} else if(actor->type == ActorType::Sphere) {
		SphereActor *sphere = (SphereActor *)actor;
		AddSphere(sphere->pos, sphere->rotation, sphere->velocity, sphere->density, sphere->radius, sphere->movementType, sphere);
	}
}

void SaveFluidPositions() {
	float minDensity = gActiveFluidScenario != NULL ? gActiveFluidScenario->render.minDensity : gActiveScene->render.minDensity;
	float *data = gPointSprites->Map();
	bool noDensity = gSSFRenderMode == SSFRenderMode::Points;
	gFluidSystem->writeToVBO(data, gTotalFluidParticles, noDensity, minDensity);
	gPointSprites->UnMap();
}

void advanceSimulation(float dtime) {
	const physx::PxReal timestep = 1.0f / 60.0f;
	while(dtime > 0.0f) {
		gScene->simulate(timestep);
		gScene->fetchResults(true);
		dtime -= timestep;
	}
}

void SingleStepPhysX(const float frametime) {
	// Advance simulation
	advanceSimulation(frametime);

	// Save fluid positions
	if(gSSFRenderMode != SSFRenderMode::Disabled)
		SaveFluidPositions();
}

CFluidSystem *CreateParticleFluidSystem() {
	FluidSimulationProperties particleSystemDesc = FluidSimulationProperties();

	particleSystemDesc.stiffness = gFluidStiffness;
	particleSystemDesc.viscosity = gFluidViscosity;

	particleSystemDesc.restitution = gFluidRestitution;
	particleSystemDesc.damping = gFluidDamping;
	particleSystemDesc.dynamicFriction = gFluidDynamicFriction;
	particleSystemDesc.particleMass = gFluidParticleMass;

	particleSystemDesc.maxMotionDistance = gFluidMaxMotionDistance;
	particleSystemDesc.restParticleDistance = gFluidRestParticleDistance;
	particleSystemDesc.restOffset = gFluidRestOffset;
	particleSystemDesc.contactOffset = gFluidContactOffset;
	particleSystemDesc.gridSize = gFluidParticleRadius * 6;

	assert(particleSystemDesc.contactOffset >= particleSystemDesc.restOffset);

	return new CFluidSystem(gPhysicsSDK, particleSystemDesc, MAX_FLUID_PARTICLES);
}

void ResetScene() {
	assert(gActiveFluidScenario != nullptr);

	printf("Load/Reload scene: %s\n", gActiveFluidScenario->name);

	// Release actors
	ClearScene();

	// Set scene properties
	gScene->setGravity(toPxVec3(gActiveFluidScenario->gravity));

	gFluidMaxMotionDistance = gActiveFluidScenario->sim.maxMotionDistance;
	gFluidContactOffset = gActiveFluidScenario->sim.contactOffset;
	gFluidRestOffset = gActiveFluidScenario->sim.restOffset;
	gFluidRestitution = gActiveFluidScenario->sim.restitution;
	gFluidDamping = gActiveFluidScenario->sim.damping;
	gFluidDynamicFriction = gActiveFluidScenario->sim.dynamicFriction;
	gFluidParticleMass = gActiveFluidScenario->sim.particleMass;

	gFluidParticleRadius = gActiveFluidScenario->sim.particleRadius;
	gFluidViscosity = gActiveFluidScenario->sim.viscosity;
	gFluidStiffness = gActiveFluidScenario->sim.stiffness;
	gFluidRestParticleDistance = gActiveFluidScenario->sim.restParticleDistance;
	gRigidBodyFallPos = gActiveFluidScenario->actorCreatePosition;
	gFluidParticleRenderFactor = gActiveFluidScenario->render.particleRenderFactor;

	// Add plane
	AddPlane();

	// Create fluid system
	gFluidSystem = CreateParticleFluidSystem();

	// Set GPU acceleration for particle fluid if supported
	gFluidSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eCOLLISION_TWOWAY, true);
	gFluidSystem->setParticleBaseFlag(physx::PxParticleBaseFlag::eGPU, gGPUDispatcher && gFluidUseGPUAcceleration);

	if(gFluidSystem) {
		physx::PxActor *fluidActor = gFluidSystem->getActor();
		gScene->addActor(*fluidActor);
		gActors.push_back(fluidActor);
	}

	// Adding actors immediately
	for(size_t i = 0, count = gActiveFluidScenario->getActorCount(); i < count; i++) {
		Actor *actor = gActiveFluidScenario->getActor(i);
		actor->timeElapsed = 0.0f;

		if(actor->time == -1) {
			AddScenarioActor(actor);
		}
	}

	// Adding waters immediately
	for(size_t i = 0; i < gActiveFluidScenario->getFluidContainerCount(); i++) {
		FluidContainer *container = gActiveFluidScenario->getFluidContainer(i);
		container->timeElapsed = 0.0f;
		container->emitterElapsed = 0.0f;
		container->emitterCoolDownElapsed = 0.0f;
		container->emitterCoolDownActive = false;

		if(container->time == -1 && !container->isEmitter && gWaterAddBySceneChange) {
			AddWater(*container, container->type);
		}
	}

	gTotalTimeElapsed = 0;

	// Simulate physx one time
	SingleStepPhysX(0.000001f);
}

void InitializePhysX() {
	std::cout << "  PhysX Version: " << PX_PHYSICS_VERSION_MAJOR << "." << PX_PHYSICS_VERSION_MINOR << "." << PX_PHYSICS_VERSION_BUGFIX << std::endl;
	std::cout << "  PhysX Foundation Version: " << PX_FOUNDATION_VERSION_MAJOR << "." << PX_FOUNDATION_VERSION_MINOR << "." << PX_FOUNDATION_VERSION_BUGFIX << std::endl;

	// Create Physics SDK
	gPhysXFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gPhysXFoundation, physx::PxTolerancesScale());

	if(gPhysicsSDK == NULL) {
		std::cerr << "  Could not create PhysX SDK, exiting!" << std::endl;
		exit(1);
	}

	// Initialize PhysX Extensions
	if(!PxInitExtensions(*gPhysicsSDK, NULL)) {
		std::cerr << "  Could not initialize PhysX extensions, exiting!" << std::endl;
		exit(1);
	}

	// Connect to visual debugg1er
#ifdef PVD_ENABLED
	gIsPhysXPvdConnected = false;
	gPhysXVisualDebugger = PxCreatePvd(*gPhysXFoundation);
	if(gPhysXVisualDebugger != NULL) {
		gPhysXPvdTransport = PxDefaultPvdSocketTransportCreate(PVD_Host, PVD_Port, 10000);
		if(gPhysXPvdTransport != NULL) {
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
	uint32_t coreCount = COSLowLevel::getInstance()->getNumCPUCores();
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

void RenderSpacedBitmapString(
	int x,
	int y,
	int spacing,
	void *font,
	char *string) {
	char *c;
	int x1 = x;

	for(c = string; *c != '\0'; c++) {
		glRasterPos2i(x1, y);
		glutBitmapCharacter(font, *c);
		x1 = x1 + glutBitmapWidth(font, *c) + spacing;
	}
}

void DrawGLCube(float hx, float hy, float hz) {
	glBegin(GL_QUADS);		// Draw The Cube Using quads

	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(hx, hy, -hz);	// Top Right Of The Quad (Top)
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(-hx, hy, -hz);	// Top Left Of The Quad (Top)
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(-hx, hy, hz);	// Bottom Left Of The Quad (Top)
	glNormal3f(0.0f, 1.0f, 0.0f); glVertex3f(hx, hy, hz);	// Bottom Right Of The Quad (Top)

	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(hx, -hy, hz);	// Top Right Of The Quad (Bottom)
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(-hx, -hy, hz);	// Top Left Of The Quad (Bottom)
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(-hx, -hy, -hz);	// Bottom Left Of The Quad (Bottom)
	glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(hx, -hy, -hz);	// Bottom Right Of The Quad (Bottom)

	glNormal3f(0.0f, 0.0, 1.0f); glVertex3f(hx, hy, hz);	// Top Right Of The Quad (Front)
	glNormal3f(0.0f, 0.0, 1.0f); glVertex3f(-hx, hy, hz);	// Top Left Of The Quad (Front)
	glNormal3f(0.0f, 0.0, 1.0f); glVertex3f(-hx, -hy, hz);	// Bottom Left Of The Quad (Front)
	glNormal3f(0.0f, 0.0, 1.0f); glVertex3f(hx, -hy, hz);	// Bottom Right Of The Quad (Front)

	glNormal3f(0.0f, 0.0, -1.0f); glVertex3f(hx, -hy, -hz);	// Top Right Of The Quad (Back)
	glNormal3f(0.0f, 0.0, -1.0f); glVertex3f(-hx, -hy, -hz);	// Top Left Of The Quad (Back)
	glNormal3f(0.0f, 0.0, -1.0f); glVertex3f(-hx, hy, -hz);	// Bottom Left Of The Quad (Back)
	glNormal3f(0.0f, 0.0, -1.0f); glVertex3f(hx, hy, -hz);	// Bottom Right Of The Quad (Back)

	glNormal3f(-1.0f, 0.0, 0.0f); glVertex3f(-hx, hy, hz);	// Top Right Of The Quad (Left)
	glNormal3f(-1.0f, 0.0, 0.0f); glVertex3f(-hx, hy, -hz);	// Top Left Of The Quad (Left)
	glNormal3f(-1.0f, 0.0, 0.0f); glVertex3f(-hx, -hy, -hz);	// Bottom Left Of The Quad (Left)
	glNormal3f(-1.0f, 0.0, 0.0f); glVertex3f(-hx, -hy, hz);	// Bottom Right Of The Quad (Left)

	glNormal3f(1.0f, 0.0, 0.0f); glVertex3f(hx, hy, -hz);	// Top Right Of The Quad (Right)
	glNormal3f(1.0f, 0.0, 0.0f); glVertex3f(hx, hy, hz);	// Top Left Of The Quad (Right)
	glNormal3f(1.0f, 0.0, 0.0f); glVertex3f(hx, -hy, hz);	// Bottom Left Of The Quad (Right)
	glNormal3f(1.0f, 0.0, 0.0f); glVertex3f(hx, -hy, -hz);	// Bottom Right Of The Quad (Right)

	glEnd();			// End Drawing The Cube
}

void DrawAxes() {
	//To prevent the view from disturbed on repaint
	//this push matrix call stores the current matrix state
	//and restores it once we are done with the arrow rendering
	glPushMatrix();
	glColor3f(0, 0, 1);
	glPushMatrix();
	glTranslatef(0, 0, 0.8f);
	glutSolidCone(0.0325, 0.2, 4, 1);
	//Draw label
	glTranslatef(0, 0.0625, 0.225f);
	RenderSpacedBitmapString(0, 0, 0, GLUT_BITMAP_HELVETICA_10, "Z");
	glPopMatrix();
	glutSolidCone(0.0225, 1, 4, 1);

	glColor3f(1, 0, 0);
	glRotatef(90, 0, 1, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0.8f);
	glutSolidCone(0.0325, 0.2, 4, 1);
	//Draw label
	glTranslatef(0, 0.0625, 0.225f);
	RenderSpacedBitmapString(0, 0, 0, GLUT_BITMAP_HELVETICA_10, "X");
	glPopMatrix();
	glutSolidCone(0.0225, 1, 4, 1);

	glColor3f(0, 1, 0);
	glRotatef(90, -1, 0, 0);
	glPushMatrix();
	glTranslatef(0, 0, 0.8f);
	glutSolidCone(0.0325, 0.2, 4, 1);
	//Draw label
	glTranslatef(0, 0.0625, 0.225f);
	RenderSpacedBitmapString(0, 0, 0, GLUT_BITMAP_HELVETICA_10, "Y");
	glPopMatrix();
	glutSolidCone(0.0225, 1, 4, 1);
	glPopMatrix();
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
		int current = glutGet(GLUT_ELAPSED_TIME);

		if(current > gFluidLatestExternalAccelerationTime) {
			gFluidSystem->setExternalAcceleration(physx::PxVec3(0.0f, 0.0f, 0.0f));
			gFluidLatestExternalAccelerationTime = -1;
		}
	}

	// Update PhysX
	if(!paused) {
		SingleStepPhysX(frametime);
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

void DrawBox(physx::PxShape *pShape) {
	glm::vec4 color = getColor(pShape->getActor(), DefaultDynamicRigidBodyCubeColor);

	physx::PxTransform pT = physx::PxShapeExt::getGlobalPose(*pShape, *pShape->getActor());
	physx::PxBoxGeometry bg;
	pShape->getBoxGeometry(bg);
	physx::PxMat33 m = physx::PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 multm = gCamera.mvp * mat;
	gRenderer->LoadMatrix(multm);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);
	DrawGLCube(bg.halfExtents.x, bg.halfExtents.y, bg.halfExtents.z);
	gLightingShader->disable();
}

void DrawSphere(physx::PxShape *pShape) {
	glm::vec4 color = getColor(pShape->getActor(), DefaultDynamicRigidBodySphereColor);

	physx::PxTransform pT = physx::PxShapeExt::getGlobalPose(*pShape, *pShape->getActor());
	physx::PxSphereGeometry sg;
	pShape->getSphereGeometry(sg);
	physx::PxMat33 m = physx::PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 multm = gCamera.mvp * mat;
	gRenderer->LoadMatrix(multm);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);
	glutSolidSphere(sg.radius, 16, 16);
	gLightingShader->disable();
}

void DrawCapsule(physx::PxShape *pShape) {
	glm::vec4 color = getColor(pShape->getActor(), DefaultDynamicRigidBodyCapsuleColor);

	physx::PxTransform pT = physx::PxShapeExt::getGlobalPose(*pShape, *pShape->getActor());
	physx::PxCapsuleGeometry cg;
	pShape->getCapsuleGeometry(cg);
	physx::PxMat33 m = physx::PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 multm = gCamera.mvp * mat;

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->ulocColor, &color[0]);

	glm::mat4 rotation = glm::rotate(multm, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 translation = glm::translate(rotation, glm::vec3(0.0f, 0.0f, -cg.halfHeight));
	gRenderer->LoadMatrix(translation);
	glutSolidCylinder(cg.radius, 2 * cg.halfHeight, 16, 16);
	glutSolidSphere(cg.radius, 16, 16);
	glm::mat4 trans2 = glm::translate(translation, glm::vec3(0.0f, 0.0f, 2.0f * cg.halfHeight));
	gRenderer->LoadMatrix(trans2);
	glutSolidSphere(cg.radius, 16, 16);

	gLightingShader->disable();
}

void DrawShape(physx::PxShape *shape) {
	physx::PxGeometryType::Enum type = shape->getGeometryType();

	switch(type) {
		case physx::PxGeometryType::eBOX:
			DrawBox(shape);
			break;

		case physx::PxGeometryType::eSPHERE:
			DrawSphere(shape);
			break;

		case physx::PxGeometryType::eCAPSULE:
			DrawCapsule(shape);
			break;
	}
}

void DrawBounds(const physx::PxBounds3 &bounds) {
	physx::PxVec3 center = bounds.getCenter();
	physx::PxVec3 scale = bounds.getDimensions();

	GLfloat mat_diffuse[4] = { 0, 1, 1, 1 };
	glColor4fv(mat_diffuse);

	glm::mat4 mvp = glm::translate(gCamera.mvp, glm::vec3(center.x, center.y, center.z));
	gRenderer->LoadMatrix(mvp);
	DrawGLCube(scale.x / 2, scale.y / 2, scale.z / 2);
}

void DrawActorBounding(physx::PxActor *actor) {
	physx::PxBounds3 bounds = actor->getWorldBounds();
	glm::vec3 min = toGLMVec3(bounds.minimum);
	glm::vec3 max = toGLMVec3(bounds.maximum);
	if(CFrustum::getInstance()->containsBounds(min, max)) {
		gDrawedActors++;
		DrawBounds(bounds);
	}
}

void DrawActor(physx::PxActor *actor) {
	physx::PxBounds3 bounds = actor->getWorldBounds();
	glm::vec3 min = toGLMVec3(bounds.minimum);
	glm::vec3 max = toGLMVec3(bounds.maximum);
	if(CFrustum::getInstance()->containsBounds(min, max)) {
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
	for(size_t index = 0; index < gActors.size(); ++index) {
		physx::PxActor *act = gActors.at(index);
		DrawActor(act);
	}
}

void RenderActorBoundings() {
	// Render all the actors in the scene as bounding volume
	for(size_t index = 0; index < gActors.size(); ++index) {
		physx::PxActor *act = gActors.at(index);
		DrawActorBounding(act);
	}
}

void ShutdownPhysX() {
	ClearScene();
	gScene->release();

	gDefaultMaterial->release();

	if(gCudaContextManager)
		gCudaContextManager->release();

#ifdef PVD_ENABLED
	if(gPhysXVisualDebugger != NULL) {
		if(gPhysXVisualDebugger->isConnected())
			gPhysXVisualDebugger->disconnect();
		gPhysXVisualDebugger->release();
	}
#endif

	gPhysicsSDK->release();
	gPhysXFoundation->release();
}

void OnReshape(int nw, int nh) {
	windowWidth = nw;
	windowHeight = nh;

	if(windowHeight < 1) windowHeight = 1;
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

const char *GetFluidProperty(unsigned int prop) {
	switch(prop) {
		case FLUID_PROPERTY_VISCOSITY:
			return "Viscosity\0";

		case FLUID_PROPERTY_STIFFNESS:
			return "Stiffness\0";

		case FLUID_PROPERTY_MAXMOTIONDISTANCE:
			return "Particle max motion distance\0";

		case FLUID_PROPERTY_CONTACTOFFSET:
			return "Particle contact offset\0";

		case FLUID_PROPERTY_RESTOFFSET:
			return "Particle rest offset\0";

		case FLUID_PROPERTY_RESTITUTION:
			return "Restitution\0";

		case FLUID_PROPERTY_DAMPING:
			return "Damping\0";

		case FLUID_PROPERTY_DYNAMICFRICTION:
			return "Dynamic friction\0";

		case FLUID_PROPERTY_PARTICLEMASS:
			return "Particle mass\0";

		case FLUID_PROPERTY_DEPTH_BLUR_SCALE:
			return "Depth blur scale\0";

		case FLUID_PROPERTY_PARTICLE_RENDER_FACTOR:
			return "Particle render factor\0";

		case FLUID_PROPERTY_DEBUGTYPE:
			return "Debug type\0";

		case FLUID_PROPERTY_COLOR_FALLOFF_SCALE:
			return "Color falloff scale\0";

		case FLUID_PROPERTY_COLOR_FALLOFF_ALPHA:
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
	if(gActiveFluidScenario) {
		// Add actors
		for(size_t i = 0, count = gActiveFluidScenario->getActorCount(); i < count; i++) {
			Actor *actor = gActiveFluidScenario->getActor(i);

			if(actor->time > 0) {
				if(actor->timeElapsed < (float)actor->time) {
					actor->timeElapsed += frametime;

					if(actor->timeElapsed >= (float)actor->time) {
						AddScenarioActor(actor);
					}
				}
			}
		}

		// Add fluids
		for(size_t i = 0, count = gActiveFluidScenario->getFluidContainerCount(); i < count; i++) {
			FluidContainer *container = gActiveFluidScenario->getFluidContainer(i);

			float time;

			if(!container->isEmitter) {
				// Einmaliger partikel emitter
				if(container->time > 0) {
					time = (float)container->time;

					if(container->timeElapsed < time) {
						container->timeElapsed += frametime;

						if(container->timeElapsed >= time) {
							AddWater(*container, container->type);
						}
					}
				}
			} else if(!gStoppedEmitter) {
				time = container->emitterTime;
				float duration = (float)container->emitterDuration;

				if(time > 0.0f) {
					container->emitterElapsed += frametime;

					if((container->emitterElapsed < duration) || (container->emitterDuration == 0)) {
						if(container->timeElapsed < time) {
							container->timeElapsed += frametime;

							if(container->timeElapsed >= time) {
								container->timeElapsed = 0.0f;
								AddWater(*container, container->type);
							}
						}
					} else if(container->emitterCoolDown > 0.0f) {
						if(!container->emitterCoolDownActive) {
							container->emitterCoolDownActive = true;
							container->emitterCoolDownElapsed = 0.0f;
						}

						if(container->emitterCoolDownActive) {
							container->emitterCoolDownElapsed += frametime;

							if(container->emitterCoolDownElapsed >= (float)container->emitterCoolDown) {
								// Cool down finished
								container->emitterCoolDownActive = false;
								container->emitterElapsed = 0.0f;
								container->timeElapsed = 0.0f;
							}
						}
					}
				}
			}
		}
	}
}

void Update(const glm::mat4 &proj, const glm::mat4 &modl, const float frametime) {
	// Update frustum
	CFrustum::getInstance()->update(&proj[0][0], &modl[0][0]);

	// Create actor based on time
	if(!paused) {
		CreateActorsBasedOnTime(frametime * 1000.0f);
	}

	// Update PhysX
	UpdatePhysX(frametime);
}

void RenderOSDLine(OSDRenderPosition &osdpos, char *value) {
	gRenderer->SetColor(0, 0, 0, 0);
	RenderSpacedBitmapString(osdpos.x, osdpos.y, -1, GLUT_BITMAP_8_BY_13, value);
	gRenderer->SetColor(1, 1, 1, 1);
	RenderSpacedBitmapString(osdpos.x + 1, osdpos.y + 1, -1, GLUT_BITMAP_8_BY_13, value);
	osdpos.newLine();
}

std::string drawingError = "";
void RenderOSD() {
	char buffer[256];

	// Setup ortho for font rendering
	glm::mat4 orthoProj = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
	glm::mat4 orthoMVP = glm::mat4(1.0f) * orthoProj;
	gRenderer->LoadMatrix(orthoMVP);

	// Disable depth testing
	gRenderer->SetDepthTest(false);

	// Set font color
	gRenderer->SetColor(1, 1, 1, 1);

	// Enable blending
	//gRenderer->SetBlending(true);

	/*
	// Enable font texture
	gRenderer->EnableTexture(0, fontTex);

	gFontShader->enable();
	gFontShader->uniformMatrix4(gFontShader->getUniformLocation("mvp"), &orthoMVP[0][0]);
	gFontShader->uniform1i(gFontShader->getUniformLocation("fontTex"), 0);

	texVertexBuffer->bind();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 9, (void*)(0));
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 9, (void*)(0 + (sizeof(float) * 3)));
	texVertexBuffer->drawElements(GL_TRIANGLES);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	texVertexBuffer->unbind();

	gFontShader->disable();

	// Disable font texture
	gRenderer->DisableTexture(0, fontTex);
	*/

	// Disable blending
	//gRenderer->SetBlending(false);

	OSDRenderPosition osdPos;
	osdPos.x = 20;
	osdPos.y = 20;

	// Render text
	sprintf_s(buffer, "FPS: %3.2f", fps);
	RenderOSDLine(osdPos, buffer);
	sprintf_s(buffer, "Show osd: %s (T)", showOSD ? "yes" : "no");
	RenderOSDLine(osdPos, buffer);

	if(showOSD) {
		sprintf_s(buffer, "Drawed actors: %zu of %zu", gDrawedActors, gTotalActors);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Total fluid particles: %lu", gTotalFluidParticles);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw error: %s", drawingError.c_str());
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Simulation state (O): %s", paused ? "paused" : "running");
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
		sprintf_s(buffer, "Fluid scenario (L): %d / %zu - %s", gActiveFluidScenarioIdx + 1, gFluidScenarios.size(), gActiveFluidScenario ? gActiveFluidScenario->name : "No scenario loaded!");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "New actor (Space)");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Reset current scene (R)");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid add acceleration (Arrow Keys)");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid using gpu acceleration (H): %s", gFluidUseGPUAcceleration ? "yes" : "no");
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
		sprintf_s(buffer, "Fluid min density: %f", gActiveFluidScenario ? gActiveFluidScenario->render.minDensity : gActiveScene->render.minDensity);
		RenderOSDLine(osdPos, buffer);
	}

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

	gSkyboxVBO->bind();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(float) * 3, (void *)(0));
	gSkyboxVBO->drawElements(GL_TRIANGLES);
	glDisableClientState(GL_VERTEX_ARRAY);
	gSkyboxVBO->unbind();

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

void RenderSceneFBO(const glm::mat4 &mvp) {
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

float appStartTime = 0.0f;
float realLatestFrameTime = 0.0f;
float fixedFrametime = 1.0f / 60.0f;
void OnRender() {
	if(!gScene) return;

	float realFrametimeStart = (float)COSLowLevel::getInstance()->getTimeMilliSeconds();

	// Calculate fps
	totalFrames++;

	if((realFrametimeStart - appStartTime) > 1000.0f) {
		float elapsedTime = float(realFrametimeStart - appStartTime);
		fps = (((float)totalFrames * 1000.0f) / elapsedTime);
		appStartTime = realFrametimeStart;
		totalFrames = 0;
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
	gCamera = CCamera(0.0f, 4.0f, dist, Deg2Rad(rX), Deg2Rad(rY), defaultZNear, defaultZFar, Deg2Rad(defaultFov), (float)windowWidth / (float)windowHeight);
	glm::mat4 mvp = gCamera.mvp;
	glm::mat4 proj = gCamera.projection;
	glm::mat4 mdlv = gCamera.modelview;
	gRenderer->LoadMatrix(mvp);

	// Update (Frustum and PhysX)
	Update(proj, mdlv, 1.0f / 60.0f);

	// Clear back buffer
	glm::vec3 backcolor = gActiveScene->backgroundColor;
	gRenderer->ClearColor(backcolor.x, backcolor.y, backcolor.z, 0.0f);
	gRenderer->Clear(ClearFlags::Color | ClearFlags::Depth);

	bool drawFluidParticles = gSSFRenderMode != SSFRenderMode::Disabled;

	// Render scene to FBO
	if(drawFluidParticles)
		RenderSceneFBO(mvp);

	// Load mvp matrix
	gRenderer->LoadMatrix(mvp);

	// Render scene
	gDrawedActors = 0;
	RenderScene(mvp);

	// Render fluid
	if(drawFluidParticles) {
		gFluidRenderer->setParticleRadius(gFluidParticleRadius * gFluidParticleRenderFactor);
		gFluidRenderer->Render(gCamera, gTotalFluidParticles, options, windowWidth, windowHeight);
	}

	// Check for opengl error
	drawingError = gRenderer->CheckError();

	// Render OSD
	RenderOSD();

	// Draw frame
	gRenderer->Flip();

	float curTime = (float)COSLowLevel::getInstance()->getTimeMilliSeconds();
	float frametime = curTime - realFrametimeStart;
	realLatestFrameTime = frametime;
	gTotalTimeElapsed += (curTime - realFrametimeStart);
}

void Mouse(int button, int s, int x, int y) {
	if(s == GLUT_DOWN) {
		oldX = x;
		oldY = y;
	}

	if(button == GLUT_RIGHT_BUTTON)
		state = 0;
	else
		state = 1;
}

void Motion(int x, int y) {
	if(state == 0)
		dist *= (1 + (y - oldY) / 60.0f);
	else {
		rY += (x - oldX) / 5.0f;
		rX += (y - oldY) / 5.0f;
	}

	oldX = x;
	oldY = y;

	glutPostRedisplay();
}

void AddActor(const ActorCreationKind kind) {
	glm::vec3 pos = gRigidBodyFallPos;
	glm::vec3 vel = gDefaultRigidBodyVelocity;
	float density = gDefaultRigidBodyDensity;
	glm::quat rotation = toGLMQuat(physx::PxQuat(Deg2Rad(RandomRadius()), physx::PxVec3(0, 1, 0)));
	switch(kind) {
		case ActorCreationKind::RigidBox:
			AddBox(pos, rotation, vel, density, glm::vec3(0.5, 0.5, 0.5), false, ActorMovementType::Dynamic, nullptr);
			break;

		case ActorCreationKind::RigidSphere:
			AddSphere(pos, rotation, vel, density, 0.5f, ActorMovementType::Dynamic, nullptr);
			break;

		case ActorCreationKind::RigidCapsule:
			AddCapsule(pos, rotation, vel, density, glm::vec2(0.5f, 1.0f), ActorMovementType::Dynamic);
			break;

		case ActorCreationKind::FluidDrop:
			AddWater(FluidType::Drop);
			break;

		case ActorCreationKind::FluidPlane:
			AddWater(FluidType::Plane);
			break;

		case ActorCreationKind::FluidCube:
			AddWater(FluidType::Box);
			break;

		case ActorCreationKind::FluidSphere:
			AddWater(FluidType::Sphere);
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
	gFluidLatestExternalAccelerationTime = glutGet(GLUT_ELAPSED_TIME) + 3000; // 3 Seconds
	gFluidSystem->setExternalAcceleration(acc);
}

void KeyDownSpecial(int key, int x, int y) {
	const float accSpeed = 10.0f;
	const physx::PxForceMode::Enum accMode = physx::PxForceMode::eACCELERATION;

	switch(key) {
		case GLUT_KEY_RIGHT:
		{
			gFluidSystem->addForce(physx::PxVec3(1.0f * accSpeed, 0.0f, 0.0f), accMode);
			break;
		}

		case GLUT_KEY_LEFT:
		{
			gFluidSystem->addForce(physx::PxVec3(-1.0f * accSpeed, 0.0f, 0.0f), accMode);
			break;
		}

		case GLUT_KEY_UP:
		{
			gFluidSystem->addForce(physx::PxVec3(0.0f, 0.0f, -1.0f * accSpeed), accMode);
			break;
		}

		case GLUT_KEY_DOWN:
		{
			gFluidSystem->addForce(physx::PxVec3(0.0f, 0.0f, 1.0f * accSpeed), accMode);
			break;
		}
	}
}

void KeyUp(unsigned char key, int x, int y) {
	switch(key) {
		case 27: // Escape
		{
			exit(0);
			break;
		}

		case 49: // 1 - 7
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		{
			int index = key - 49;
			assert(index >= 0 && index <= (int)ActorCreationKind::Max);
			gCurrentActorCreationKind = (ActorCreationKind)index;
			break;
		}
		case 102: // f
		{
			windowFullscreen = !windowFullscreen;
			if(windowFullscreen) {
				lastWindowPosX = glutGet(GLUT_WINDOW_X);
				lastWindowPosY = glutGet(GLUT_WINDOW_Y);
				lastWindowWidth = windowWidth;
				lastWindowHeight = windowHeight;
				glutFullScreen();
			} else {
				glutPositionWindow(lastWindowPosX, lastWindowPosY);
				glutReshapeWindow(lastWindowWidth, lastWindowHeight);
			}
			break;
		}
		case 114: // r
		{
			ResetScene();
			break;
		}

		case 116: // t
		{
			showOSD = !showOSD;
			break;
		}

		case 98: // b
		{
			gDrawBoundBox = !gDrawBoundBox;
			break;
		}

		case 100: // d
		{
			gHideRigidBodies++;

			if(gHideRigidBodies > HideRigidBody_MAX)
				gHideRigidBodies = 0;

			break;
		}

		case 119: // w
		{
			gDrawWireframe = !gDrawWireframe;
			break;
		}

		case 118: // v
		{
			gFluidCurrentProperty++;

			if(gFluidCurrentProperty > MAX_FLUID_PROPERTY) gFluidCurrentProperty = FLUID_PROPERTY_NONE;

			break;
		}

		case 104: // h
		{
			ToggleFluidGPUAcceleration();
			break;
		}

		case 107: // k
		{
			gStoppedEmitter = !gStoppedEmitter;
			break;
		}

		case 108: // l
		{
			if(gFluidScenarios.size() > 0) {
				gActiveFluidScenarioIdx++;

				if(gActiveFluidScenarioIdx > (int)gFluidScenarios.size() - 1) gActiveFluidScenarioIdx = 0;

				gActiveFluidScenario = gFluidScenarios[gActiveFluidScenarioIdx];
				ResetScene();
			}

			break;
		}

		case 109: // m
		{
			gSSFBlurActive = !gSSFBlurActive;
			break;
		}

		case 110: // n
		{
			gWaterAddBySceneChange = !gWaterAddBySceneChange;
			break;
		}

		case 115: // s
		{
			gFluidDebugType = FluidDebugType::Final;

			int mode = (int)gSSFRenderMode;

			mode++;
			if(mode > (int)SSFRenderMode::Disabled) mode = (int)SSFRenderMode::Fluid;
			gSSFRenderMode = (SSFRenderMode)mode;

			SingleStepPhysX(0.000001f);
			break;
		}

		case 99: // c
		{
			gSSFCurrentFluidIndex++;

			if(gSSFCurrentFluidIndex > gActiveScene->getFluidColorCount() - 1) gSSFCurrentFluidIndex = 0;

			break;
		}

		case 111: // o
		{
			paused = !paused;
			break;
		}

		case 112: // p
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
		case FLUID_PROPERTY_VISCOSITY:
		{
			gFluidViscosity += value;
			gFluidSystem->setViscosity(gFluidViscosity);
			gActiveFluidScenario->sim.viscosity = gFluidViscosity;
		} break;

		case FLUID_PROPERTY_STIFFNESS:
		{
			gFluidStiffness += value;
			gFluidSystem->setStiffness(gFluidStiffness);
			gActiveFluidScenario->sim.stiffness = gFluidStiffness;
		} break;

		case FLUID_PROPERTY_MAXMOTIONDISTANCE:
		{
			gFluidMaxMotionDistance += value / 1000.0f;
			gFluidSystem->setMaxMotionDistance(gFluidMaxMotionDistance);
			gActiveScene->sim.maxMotionDistance = gFluidMaxMotionDistance;
		} break;

		case FLUID_PROPERTY_CONTACTOFFSET:
		{
			gFluidContactOffset += value / 1000.0f;
			gFluidSystem->setContactOffset(gFluidContactOffset);
			gActiveScene->sim.contactOffset = gFluidContactOffset;
		} break;

		case FLUID_PROPERTY_RESTOFFSET:
		{
			gFluidRestOffset += value / 1000.0f;
			gFluidSystem->setRestOffset(gFluidRestOffset);
			gActiveScene->sim.restOffset = gFluidRestOffset;
		} break;

		case FLUID_PROPERTY_RESTITUTION:
		{
			gFluidRestitution += value / 1000.0f;
			gFluidSystem->setRestitution(gFluidRestitution);
			gActiveScene->sim.restitution = gFluidRestitution;
		} break;

		case FLUID_PROPERTY_DAMPING:
		{
			gFluidDamping += value / 1000.0f;
			gFluidSystem->setDamping(gFluidDamping);
			gActiveScene->sim.damping = gFluidDamping;
		} break;

		case FLUID_PROPERTY_DYNAMICFRICTION:
		{
			gFluidDynamicFriction += value / 1000.0f;
			gFluidSystem->setDynamicFriction(gFluidDynamicFriction);
			gActiveScene->sim.dynamicFriction = gFluidDynamicFriction;
		} break;

		case FLUID_PROPERTY_PARTICLEMASS:
		{
			gFluidParticleMass += value / 1000.0f;
			gFluidSystem->setParticleMass(gFluidParticleMass);
			gActiveScene->sim.particleMass = gFluidParticleMass;
		} break;

		case FLUID_PROPERTY_DEPTH_BLUR_SCALE:
		{
			gSSFBlurDepthScale += value / 10000.0f;
		} break;

		case FLUID_PROPERTY_PARTICLE_RENDER_FACTOR:
		{
			gFluidParticleRenderFactor += value / 10.0f;
			gFluidParticleRenderFactor = roundFloat(gFluidParticleRenderFactor);
		} break;

		case FLUID_PROPERTY_DEBUGTYPE:
		{
			int inc = (int)value;

			int debugType = (int)gFluidDebugType;
			debugType += inc;

			if(debugType < 0) debugType = (int)FluidDebugType::Max;
			if(debugType > (int)FluidDebugType::Max) debugType = (int)FluidDebugType::Final;

			gFluidDebugType = (FluidDebugType)debugType;
		} break;

		case FLUID_PROPERTY_COLOR_FALLOFF_SCALE:
		{
			FluidColor &activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);
			activeFluidColor.falloffScale += value / 100.0f;
		} break;

		case FLUID_PROPERTY_COLOR_FALLOFF_ALPHA:
		{
			FluidColor &activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);
			activeFluidColor.falloff.w += value / 100.0f;
		} break;
	}
}

void KeyDown(unsigned char key, int x, int y) {
	switch(key) {
		case 32: // Space
		{
			AddActor(gCurrentActorCreationKind);
			break;
		}

		case 43: // +
		{
			// Increase fluid property
			ChangeFluidProperty(1.0f);
			break;
		}

		case 45: // -
		{
			// Decrease fluid property
			ChangeFluidProperty(-1.0f);
			break;
		}

		default:
			break;
	}
}

void OnIdle() {
	glutPostRedisplay();
}

void LoadFluidScenarios() {
	// Load scenarios
	std::vector<std::string> scenFiles = COSLowLevel::getInstance()->getFilesInDirectory("scenarios\\*.xml");

	for(unsigned int i = 0; i < scenFiles.size(); i++) {
		std::string filename = "scenarios\\";
		filename += scenFiles[i];
		FluidScenario *scenario = FluidScenario::load(filename.c_str(), gActiveScene);
		gFluidScenarios.push_back(scenario);
	}

	if(gFluidScenarios.size() > 0) {
		gActiveFluidScenarioIdx = 0;
		gActiveFluidScenario = gFluidScenarios[gActiveFluidScenarioIdx];
	} else {
		gActiveFluidScenarioIdx = -1;
		gActiveFluidScenario = NULL;
		std::cerr << "  No fluid scenario found, fluid cannot be used!" << std::endl;
	}
}

void printOpenGLInfos() {
	printf("  OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("  OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("  OpenGL Version: %s\n", glGetString(GL_VERSION));

	if(GLEW_VERSION_2_0)
		printf("  GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if(glewIsSupported("GL_ARB_framebuffer_object")) {
		int temp;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &temp);
		printf("  OpenGL FBO Max Color Attachments: %d\n", temp);
		glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &temp);
		printf("  OpenGL FBO Max Render Buffer Size: %d\n", temp);
	}
}

void drawPlane(float size) {
	float hs = size / 2.0f;
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(hs, 0.0f, -hs);
	glNormal3f(0.0f, 1.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-hs, 0.0f, -hs);
	glNormal3f(0.0f, 1.0f, 0.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-hs, 0.0f, hs);
	glNormal3f(0.0f, 1.0f, 0.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(hs, 0.0f, hs);
	glEnd();
}

void initResources() {
	// Create texture manager
	printf("  Create texture manager\n");
	gTexMng = new CTextureManager();
	gSkyboxCubemap = gTexMng->addCubemap("skybox", "textures\\skybox_texture.jpg");
	gTestTexture = gTexMng->add2D("test", "textures\\Pond.jpg");

	// Create shader manager
	gShaderMng = new CGLSLManager();

	// Create scene
	printf("  Load scene\n");
	gActiveScene = new CScene(
		FluidSimulationProperties::DefaultParticleRadius,
		FluidSimulationProperties::DefaultViscosity,
		FluidSimulationProperties::DefaultStiffness,
		FluidSimulationProperties::DefaultParticleRestDistanceFactor,
		FluidRenderProperties::DefaultParticleRenderFactor,
		FluidRenderProperties::DefaultMinDensity,
		gDefaultRigidBodyDensity);
	gActiveScene->load("scene.xml");
	gFluidParticleRadius = gActiveScene->sim.particleRadius;
	gFluidParticleRenderFactor = gActiveScene->render.particleRenderFactor;
	gSSFCurrentFluidIndex = gActiveScene->fluidColorDefaultIndex;

	// Create spherical point sprites
	printf("  Allocate spherical point sprites\n");
	gPointSprites = new CSphericalPointSprites();
	gPointSprites->Allocate(MAX_FLUID_PARTICLES);

	// Create spherical point sprites shader
	printf("  Load spherical point sprites shader\n");
	gPointSpritesShader = new CPointSpritesShader();
	Utils::attachShaderFromFile(gPointSpritesShader, GL_VERTEX_SHADER, "shaders\\PointSprites.vertex", "    ");
	Utils::attachShaderFromFile(gPointSpritesShader, GL_FRAGMENT_SHADER, "shaders\\PointSprites.fragment", "    ");

	// Create scene FBO
	printf("  Create scene FBO\n");
	gSceneFBO = new CSceneFBO(windowWidth, windowHeight);
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
	gFluidRenderer = new CScreenSpaceFluidRendering(windowWidth, windowHeight, gFluidParticleRadius * 2.0f);
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
	gSkyboxVBO = Primitives::createCube(100, 100, 100, false);
	gSkyboxShader = new CSkyboxShader();
	Utils::attachShaderFromFile(gSkyboxShader, GL_VERTEX_SHADER, "shaders\\Skybox.vertex", "    ");
	Utils::attachShaderFromFile(gSkyboxShader, GL_FRAGMENT_SHADER, "shaders\\Skybox.fragment", "    ");
}

void ReleaseResources() {
	printf("  Release skybox\n");

	if(gSkyboxShader)
		delete gSkyboxShader;

	if(gSkyboxVBO)
		delete gSkyboxVBO;

	if(gLightingShader)
		delete gLightingShader;

	printf("  Release fluid renderer\n");

	if(gFluidRenderer)
		delete gFluidRenderer;

	printf("  Release ortho shader\n");

	if(gSceneShader)
		delete gSceneShader;

	// Release scene FBO
	printf("  Release scene fbo\n");

	if(gSceneFBO)
		delete gSceneFBO;

	// Release point sprites shader
	printf("  Release spherical point sprites\n");

	if(gPointSpritesShader)
		delete gPointSpritesShader;

	if(gPointSprites)
		delete gPointSprites;

	// Release scene
	printf("  Release scene\n");

	if(gActiveScene)
		delete gActiveScene;

	// Release shader manager
	printf("  Release shader manager\n");

	if(gShaderMng)
		delete gShaderMng;

	// Release texture manager
	printf("  Release texture manager\n");

	if(gTexMng)
		delete gTexMng;
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
		FluidScenario *scen = gFluidScenarios[i];
		delete scen;
	}

	gFluidScenarios.clear();

	// Release renderer
	printf("Release Renderer\n");

	if(gRenderer)
		delete gRenderer;

	// Release singletons
	CFrustum::releaseInstance();
	COSLowLevel::releaseInstance();
}

void main(int argc, char **argv) {
	printf("%s v%s\n", APPLICATION_NAME, APPLICATION_VERSION);
	printf("%s\n", APPLICATION_COPYRIGHT);
	printf("\n");

	// Get application path
	appPath = COSLowLevel::getInstance()->getAppPath(argc, argv);

	// Initialize random generator
	srand((unsigned int)time(NULL));

	// Initialize glut window
	printf("Initialize Window\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow(APPTITLE.c_str());
	HWND hwnd = FindWindow(L"GLUT", L"ogl");
	SetWindowPos(hwnd, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_NOREPOSITION | SWP_NOSIZE);
	glutCloseFunc(OnShutdown);
	lastWindowPosX = glutGet(GLUT_WINDOW_X);
	lastWindowPosY = glutGet(GLUT_WINDOW_Y);
	if(windowFullscreen) {
		glutFullScreen();
	}

	// Initialize glew
	if(glewInit() != GLEW_OK) {
		std::cerr << "Could not initialize glew!" << std::endl;
		exit(1);
	}

	// Print OpenGL stuff
	printOpenGLInfos();

	// Required extensions check
	printf("  Checking opengl requirements...");

	int maxColorAttachments = 0;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);

	if((!GLEW_VERSION_2_0) ||
		(!glewIsSupported("GL_ARB_texture_float GL_ARB_point_sprite GL_ARB_framebuffer_object")) ||
		(maxColorAttachments < 4)) {
		printf("failed\n");
		std::cerr << std::endl << "Your graphics adapter is not supported, press any key to exit!" << std::endl;
		std::cerr << "Required opengl version:" << std::endl;
		std::cerr << "  OpenGL version 2.0 or higher" << std::endl;
		std::cerr << "Required opengl extensions:" << std::endl;
		std::cerr << "  GL_ARB_texture_float" << std::endl;
		std::cerr << "  GL_ARB_point_sprite" << std::endl;
		std::cerr << "  GL_ARB_framebuffer_object" << std::endl;
		std::cerr << "Required constants:" << std::endl;
		std::cerr << "  GL_MAX_COLOR_ATTACHMENTS >= 4" << std::endl;
		getchar();
		exit(1);
	} else
		printf("ok\n");

	wglSwapIntervalEXT(1);

	glutDisplayFunc(OnRender);
	glutIdleFunc(OnIdle);
	glutReshapeFunc(OnReshape);

	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutKeyboardUpFunc(KeyUp);
	glutKeyboardFunc(KeyDown);
	glutSpecialFunc(KeyDownSpecial);

	printf("Initialize Renderer\n");
	gRenderer = new CRenderer();

	printf("Initialize Resources\n");
	initResources();

	printf("Initialize PhysX\n");
	InitializePhysX();

	printf("Load Fluid Scenarios\n");
	LoadFluidScenarios();

	printf("Load Fluid Scenario\n");
	ResetScene();

	printf("Main loop\n");
	glutMainLoop();
}
