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

	- Cache shader uniform locations to improve performance

	- Do not use any std::map or std::string classes while running the simulation.
	  See CFBO::textures
	  Use numeric ids instead and simply map it to a static array with fixed size
	  More than 8 texture render targets are not possible anyway

	- Do not use std::vector, simple static or growable arrays will do (We never remove from a array - only clear)

	- Move away from classes and use more structs, too much useless OOP

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
#define _USE_MATH_DEFINES
#include <math.h>
#include <typeinfo>

using namespace std;

// OpenGL (Glut + Glew)
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/freeglut.h>
// OpenGL required libs
#pragma comment(lib, "glew32.lib")

// OpenGL mathmatics
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

// PhysX API
#include <PxPhysicsAPI.h>
using namespace physx;

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
using namespace rapidxml;

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
#include "Actor.h"
#include "CubeActor.h"
#include "Scene.h"
#include "FluidSystem.h"
#include "Primitives.h"
#include "TextureManager.h"
#include "FluidDescription.h"
#include "Light.h"
#include "GLSLManager.h"
#include "TextureIDs.h"

// App path
string appPath = "";

// Application
const char *APPLICATION_NAME = "Fluid Sandbox";
const char *APPLICATION_VERSION = "1.8.0";
const char *APPLICATION_AUTHOR = "Torsten Spaete";
const char *APPLICATION_COPYRIGHT = "© 2015-2021 Torsten Spaete - All rights reserved";
const string APPTITLE = APPLICATION_NAME + string(" v") + APPLICATION_VERSION + string(" by ") + APPLICATION_AUTHOR;

// Math stuff
const float DEG2RAD = (float)M_PI / 180.0f;

// PhysX Debugger
const char *PVD_Host = "localhost";
const int PVD_Port = 5425;

// PhysX SDK & Scene
static PxFoundation *gPhysXFoundation = NULL;
static PxPhysics *gPhysicsSDK = NULL;
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;
static PxMaterial *gDefaultMaterial = NULL;
static PxScene *gScene = NULL;
#ifdef PVD_ENABLED
static PxPvd *gPhysXVisualDebugger = NULL;
static PxPvdTransport *gPhysXPvdTransport = NULL;
static bool gIsPhysXPvdConnected = false;
#endif

// For GPU Acceleration
static PxGpuDispatcher *gGPUDispatcher = NULL;
static PxCudaContextManager *gCudaContextManager = NULL;

// Window vars
bool windowFullscreen = false;
int windowWidth = 1280;
int windowHeight = 720;
int lastWindowWidth = windowWidth;
int lastWindowHeight = windowHeight;
int lastWindowPosX = 0;
int lastWindowPosY = 0;
const float defaultFov = 60.0;
const float defaultZNear = 0.1f;
const float defaultZFar = 1000.0f;

// PhysX Simulation step 60 steps per second
PxReal gTimeSimulationStep = 1.0f / 60.0f;
bool gStoppedEmitter = false;

// List of added actors and rigid bodies settings
vector<PxActor *> gActors;
const float gDefaultRigidBodyDensity = 0.05f;

// Current geometry type
unsigned int gGeoType = 7;

const int HideRigidBody_None = 0;
const int HideRigidBody_Blending = 1;
const int HideRigidBody_All = 2;
const int HideRigidBody_MAX = HideRigidBody_All;

// Various drawing states
bool gDrawWireframe = false;
bool gDrawBoundBox = false;
int gHideRigidBodies = HideRigidBody_None;
bool showOSD = false;

// Drawing statistics
int gTotalActors = 0;
int gDrawedActors = 0;
unsigned int gTotalFluidParticles = 0;

// For fluid simulation
const int MAX_FLUID_PARTICLES = 512000;
CFluidSystem *gFluidSystem = NULL;
float gFluidParticleRadius = 0.05f;
float gDefaultFluidParticleFactor = 2.0f;
float gFluidParticleRenderFactor = 1.5f;
float gFluidParticleDistance = gFluidParticleRadius * gDefaultFluidParticleFactor;
PxVec3 gRigidBodyFallPos(0.0f, 10.0f, 0.0f);

// Debug types
#define	SWOWTYPE_FINAL 0
#define	SWOWTYPE_DEPTH 1
#define	SWOWTYPE_NORMAL 2
#define	SWOWTYPE_COLOR 3
#define	SWOWTYPE_DIFFUSE 4
#define	SWOWTYPE_SPECULAR 5
#define	SWOWTYPE_DIFFUSE_SPECULAR 6
#define	SWOWTYPE_SCENE 7
#define	SWOWTYPE_FRESNEL 8
#define	SWOWTYPE_REFRACTION 9
#define SWOWTYPE_REFLECTION 10
#define	SWOWTYPE_FRESNEL_REFLECTION 11
#define	SWOWTYPE_THICKNESS 12
#define	SWOWTYPE_ABSORBTION 13
const int MAX_DEBUGTYPE = SWOWTYPE_ABSORBTION;
int gFluidDebugType = SWOWTYPE_FINAL;

static CSphericalPointSprites *gPointSprites = NULL;
static CGLSL *gPointSpritesShader = NULL;
static CGLSL *gLightingShader = NULL;

bool gFluidUseGPUAcceleration = false;

const float DefaultFluidParticleDistanceFactor = 2.0f;
const float DefaultFluidParticleRenderFactor = 1.0f;
const float DefaultFluidParticleRadius = 0.05f;
const float DefaultFluidParticleMinDensity = 0.01f;
const float DefaultFluidViscosity = 10.0f;
const float DefaultFluidStiffness = 50.0f;

// 45 - 60 nvidia, 80 - 40 is better for this, 20 - 35 is a good value for water
float gFluidViscosity = DefaultFluidViscosity;
float gFluidStiffness = DefaultFluidStiffness;
float gFluidRestOffset = 0.0f;
float gFluidContactOffset = 0.0f;
float gFluidRestParticleDistance = 0.0f;
float gFluidMaxMotionDistance = 0.0f;
float gFluidRestitution = 0.0f;
float gFluidDamping = 0.0f;
float gFluidDynamicFriction = 0.0f;
float gFluidParticleMass = 0.0f;

vector<CFluidScenario *> gFluidScenarios;
CFluidScenario *gActiveFluidScenario = NULL;
int gActiveFluidScenarioIdx = -1;

// Fluid modification
int gFluidLatestExternalAccelerationTime = -1;

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
unsigned int gFluidCurrentProperty = FLUID_PROPERTY_NONE;

// For mouse dragging
int oldX = 0, oldY = 0;
float rX = 15, rY = 0;
float fps = 0;
//int startTime=0;
int totalFrames = 0;
int state = 1;
float dist = 15;
int lastFrameTime = 0;

// Renderer
static CRenderer *gRenderer = NULL;

// Fluid Renderer
static CScreenSpaceFluidRendering *gFluidRenderer = NULL;
int gSSFRenderMode = SSFRenderMode_Points;
float gSSFDetailFactor = 1.0f;
float gSSFBlurDepthScale = 0.0008f;
bool gSSFBlurActive = true;
bool gWaterAddBySceneChange = true;

// Managers
CTextureManager *gTexMng = NULL;
CGLSLManager *gShaderMng = NULL;

// Current fluid color index
int gSSFCurrentFluidIndex = 0;

// Current scene
CScene *gActiveScene = NULL;

// Current camera
CCamera gCamera;

// Non fluid rendering
CFBO *gSceneFBO = NULL;
CGLSL *gSceneShader = NULL;
CVBO *gSkyboxVBO = NULL;
CGLSL *gSkyboxShader = NULL;
CTexture *gSkyboxCubemap = NULL;
CTexture *gTestTexture = NULL;

// Timing
float gTotalTimeElapsed = 0;
bool paused = false;

// Default colors
PxVec4 DefaultStaticRigidBodyColor(0.0f, 0.0f, 0.1f, 0.3f);
PxVec4 DefaultDynamicRigidBodyCubeColor(0.85f, 0.0f, 0.0f, 1.0f);
PxVec4 DefaultDynamicRigidBodySphereColor(0, 0.85f, 0.0f, 1.0f);
PxVec4 DefaultDynamicRigidBodyCapsuleColor(0.85f, 0.85f, 0.0f, 1.0f);

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

PxQuat createQuatRotation(const PxVec3 &rotate) {
	PxQuat rot = PxQuat(rotate.x, PxVec3(1.0f, 0.0f, 0.0f));
	rot *= PxQuat(rotate.y, PxVec3(0.0f, 1.0f, 0.0f));
	rot *= PxQuat(rotate.z, PxVec3(0.0f, 0.0f, 1.0f));
	return rot;
}

void AddBox(PxVec3 dimensions, CCubeActor *cubeactor) {
	// Create box
	PxReal density = cubeactor ? cubeactor->getDensitiy() : gDefaultRigidBodyDensity;
	PxTransform transform(cubeactor ? cubeactor->getPos() : gRigidBodyFallPos,
		cubeactor ? createQuatRotation(cubeactor->getRotate()) : PxQuat(Deg2Rad(RandomRadius()), PxVec3(0, 1, 0)));
	PxBoxGeometry geometry(dimensions);

	PxRigidDynamic *nactor = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial, density);
	PxRigidBodyExt::updateMassAndInertia(*nactor, density);
	nactor->userData = cubeactor;
	nactor->setAngularDamping(0.75);
	nactor->setLinearVelocity(cubeactor ? cubeactor->getVelocity() : PxVec3(0, 0, 0));

	if(!nactor)
		cerr << "create box actor failed!" << endl;

	gScene->addActor(*nactor);

	gActors.push_back(nactor);
}

string vecToString(const PxVec3 &p) {
	char str[255];
	sprintf_s(str, "%f, %f, %f", p.x, p.y, p.z);
	return str;
}

void setActorDrain(PxActor *actor, CActor *cactor) {
	if(!cactor->getParticleDrain()) return;

	PxType actorType = actor->getConcreteType();
	if(actorType == PxConcreteType::eRIGID_STATIC || actorType == PxConcreteType::eRIGID_DYNAMIC) {
		PxRigidActor *rigActor = (PxRigidActor *)actor;
		PxU32 nShapes = rigActor->getNbShapes();
		PxShape **shapes = new PxShape * [nShapes];
		rigActor->getShapes(shapes, nShapes);

		while(nShapes--) {
			PxShapeFlags flags = shapes[nShapes]->getFlags();
			flags |= PxShapeFlag::ePARTICLE_DRAIN;
			shapes[nShapes]->setFlags(flags);
		}

		delete[] shapes;
	}
}

void AddBoxStatic(CCubeActor *cubeActor) {
	assert(cubeActor != NULL);

	// Create static box
	PxTransform transform(cubeActor->getPos(), createQuatRotation(cubeActor->getRotate()));
	PxBoxGeometry geometry(cubeActor->getSize());

	PxRigidStatic *nactor = PxCreateStatic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial);
	nactor->userData = cubeActor;

	if(!nactor)
		cerr << "create box static actor failed!" << endl;

	gScene->addActor(*nactor);
	setActorDrain(nactor, cubeActor);

	gActors.push_back(nactor);
}

void AddSphereStatic(CSphereActor *sphereActor) {
	// Create static sphere
	PxTransform transform(sphereActor ? sphereActor->getPos() : gRigidBodyFallPos,
		sphereActor ? createQuatRotation(sphereActor->getRotate()) : PxQuat(Deg2Rad(RandomRadius()), PxVec3(0, 1, 0)));
	PxSphereGeometry geometry(sphereActor ? sphereActor->getRadius() : 0.5f);

	PxRigidStatic *nactor = PxCreateStatic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial);
	nactor->userData = sphereActor;

	if(!nactor)
		cerr << "create sphere static actor failed!" << endl;

	gScene->addActor(*nactor);

	gActors.push_back(nactor);
}

void AddSphere(CSphereActor *sphereActor) {
	// Create sphere
	PxReal density = sphereActor ? sphereActor->getDensitiy() : gDefaultRigidBodyDensity;
	PxTransform transform(sphereActor ? sphereActor->getPos() : gRigidBodyFallPos,
		sphereActor ? createQuatRotation(sphereActor->getRotate()) : PxQuat(PxIdentity));
	PxSphereGeometry geometry(sphereActor ? sphereActor->getRadius() : 0.5f);

	PxRigidDynamic *nactor = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial, density);
	PxRigidBodyExt::updateMassAndInertia(*nactor, density);
	nactor->setAngularDamping(0.75);
	nactor->setLinearVelocity(sphereActor ? sphereActor->getVelocity() : PxVec3(0, 0, 0));
	nactor->userData = sphereActor;

	if(!nactor)
		cerr << "create sphere actor failed!" << endl;

	gScene->addActor(*nactor);

	gActors.push_back(nactor);
}

void AddCapsule() {
	// Create capsule
	PxReal density = gDefaultRigidBodyDensity;
	PxTransform transform(gRigidBodyFallPos, PxQuat(Deg2Rad(RandomRadius()), PxVec3(0, 1, 0)));
	PxCapsuleGeometry geometry(0.5, 1.0);

	PxRigidDynamic *nactor = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *gDefaultMaterial, density);
	PxRigidBodyExt::updateMassAndInertia(*nactor, density);
	nactor->setAngularDamping(0.75);
	nactor->setLinearVelocity(PxVec3(0, 0, 0));

	if(!nactor)
		cerr << "create capsule actor failed!" << endl;

	gScene->addActor(*nactor);

	gActors.push_back(nactor);
}

bool PointInSphere(const PxVec3 &spherePos, const float &sphereRadius, const PxVec3 point, const float particleRadius) {
	PxVec3 distance = spherePos - point;
	float length = distance.magnitude();
	float sumRadius = sphereRadius + particleRadius;
	return length <= sumRadius;
}

void AddWater(FluidContainer *container, const FluidType type) {
	unsigned int numParticles = 0;
	vector<PxVec3> particlePositionBuffer;
	vector<PxVec3> particleVelocityBuffer;

	float distance = gFluidParticleDistance;

	if(container) {

		float centerX = container->Pos.x;
		float centerY = container->Pos.y;
		float centerZ = container->Pos.z;

		float sizeX = container->Size.x;
		float sizeY = container->Size.y;
		float sizeZ = container->Size.z;

		if(type == FluidTypeSphere) {
			if(container->Radius < 0.00001f)
				container->Radius = ((sizeX + sizeY + sizeZ) / 3.0f) / 2.0f;
		}

		long numX = (int)(container->Size.x / distance);
		long numY = (int)(container->Size.y / distance);
		long numZ = (int)(container->Size.z / distance);

		float dX = distance * numX;
		float dY = distance * numY;
		float dZ = distance * numZ;

		int idx;

		if(type == FluidTypeDrop)  // Single drop
		{
			numParticles++;
			particlePositionBuffer.push_back(PxVec3(centerX, centerY, centerZ));
			particleVelocityBuffer.push_back(container->Vel);
		} else if(type == FluidTypeWall)  // Water quad
		{
			float zpos = centerZ - (dZ / 2.0f);
			idx = 0;

			for(int z = 0; z < numZ; z++) {
				float xpos = centerX - (dX / 2.0f);

				for(int x = 0; x < numX; x++) {
					numParticles++;
					particlePositionBuffer.push_back(PxVec3(xpos, centerY, zpos));
					particleVelocityBuffer.push_back(container->Vel);
					idx++;
					xpos += distance;
				}

				zpos += distance;
			}
		} else if(type == FluidTypeBlob)  // Water blob
		{
			float zpos = centerZ - (dZ / 2.0f);
			idx = 0;

			for(int z = 0; z < numZ; z++) {
				float ypos = centerY - (dY / 2.0f);

				for(int y = 0; y < numY; y++) {
					float xpos = centerX - (dX / 2.0f);

					for(int x = 0; x < numX; x++) {
						numParticles++;
						particlePositionBuffer.push_back(PxVec3(xpos, ypos, zpos));
						particleVelocityBuffer.push_back(container->Vel);
						idx++;
						xpos += distance;
					}

					ypos += distance;
				}

				zpos += distance;
			}
		} else if(type == FluidTypeSphere)  // Water sphere
		{
			PxVec3 center = PxVec3(centerX, centerY, centerZ);

			float radius = container->Radius;
			float zpos = centerZ - (dZ / 2.0f);
			idx = 0;

			for(int z = 0; z < numZ; z++) {
				float ypos = centerY - (dY / 2.0f);

				for(int y = 0; y < numY; y++) {
					float xpos = centerX - (dX / 2.0f);

					for(int x = 0; x < numX; x++) {
						PxVec3 point = PxVec3(xpos, ypos, zpos);

						if(PointInSphere(center, radius, point, gFluidParticleRadius)) {
							numParticles++;
							particlePositionBuffer.push_back(point);
							particleVelocityBuffer.push_back(container->Vel);
							idx++;
						}

						xpos += distance;
					}

					ypos += distance;
				}

				zpos += distance;
			}
		}
	}

	if(numParticles > 0) {
		gFluidSystem->createParticles(numParticles, &particlePositionBuffer[0], &particleVelocityBuffer[0]);
	}
}

void AddWater(FluidType waterType) {
	if(!gActiveFluidScenario) return;

	for(int i = 0; i < gActiveFluidScenario->getFluidContainers(); i++) {
		FluidContainer *container = gActiveFluidScenario->getFluidContainer(i);

		if(container->Time <= 0)
			AddWater(container, waterType);
	}
}

void AddPlane() {
	// Create ground plane
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));

	PxRigidStatic *plane = gPhysicsSDK->createRigidStatic(pose);

	if(!plane)
		cerr << "create plane failed!" << endl;

	PxShape *shape = plane->createShape(PxPlaneGeometry(), *gDefaultMaterial);

	if(!shape)
		cerr << "create shape failed!" << endl;

	gScene->addActor(*plane);
	gActors.push_back(plane);
}

void ClearScene() {
	// Remove fluid system
	if(gFluidSystem)
		delete gFluidSystem;

	// Remove actrors
	for(long index = 0; index < (long)gActors.size(); ++index) {
		PxActor *act = gActors.at(index);
		gScene->removeActor(*act);
		act->release();
	}

	gActors.clear();

	// Reset drawing statistics
	//gTotalFluidParticles = 0;
}

void AddScenarioActor(CActor *actor) {
	if(actor->getPrimitive() == ActorPrimitiveCube) {
		CCubeActor *cube = (CCubeActor *)actor;

		if(actor->getType() == ActorTypeStatic) {
			AddBoxStatic(cube);
		} else if(actor->getType() == ActorTypeDynamic) {
			AddBox(cube->getSize(), cube);
		}
	} else if(actor->getPrimitive() == ActorPrimitiveSphere) {
		CSphereActor *sphere = (CSphereActor *)actor;

		if(actor->getType() == ActorTypeStatic)
			AddSphereStatic(sphere);
		else if(actor->getType() == ActorTypeDynamic)
			AddSphere(sphere);
	}
}

void SaveFluidPositions() {
	float minDensity = gActiveFluidScenario != NULL ? gActiveFluidScenario->getParticleMinDensity() : gActiveScene->getFluidParticleMinDensity();
	float *data = gPointSprites->Map();
	gFluidSystem->writeToVBO(data, gTotalFluidParticles, gSSFRenderMode == SSFRenderMode_Points, minDensity);
	gPointSprites->UnMap();
}

void advanceSimulation(float dtime) {
	const PxReal timestep = 1.0f / 60.0f;
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
	if(gSSFRenderMode != SSFRenderMode_Disabled)
		SaveFluidPositions();
}

CFluidSystem *CreateParticleFluidSystem() {
	gFluidMaxMotionDistance = gActiveScene->getFluidMaxMotionDistance();
	gFluidContactOffset = gActiveScene->getFluidContactOffset();
	gFluidRestOffset = gActiveScene->getFluidRestOffset();
	gFluidRestParticleDistance = gFluidParticleDistance;
	gFluidRestitution = gActiveScene->getFluidRestitution();
	gFluidDamping = gActiveScene->getFluidDamping();
	gFluidDynamicFriction = gActiveScene->getFluidDynamicFriction();
	gFluidParticleMass = gActiveScene->getFluidParticleMass();

	// Create fluid particle system
	//PxParticleFluidDesc particleSystemDesc(gPhysicsSDK->getTolerancesScale());
	FluidDescription particleSystemDesc;

	particleSystemDesc.maxParticles = MAX_FLUID_PARTICLES;

	particleSystemDesc.stiffness = gFluidStiffness;
	particleSystemDesc.viscosity = gFluidViscosity;

	particleSystemDesc.restitution = gFluidRestitution; // 0.3
	particleSystemDesc.damping = gFluidDamping; // 0.0
	particleSystemDesc.dynamicFriction = gFluidDynamicFriction; // 0.001
	particleSystemDesc.particleMass = gFluidParticleMass;

	particleSystemDesc.maxMotionDistance = gFluidMaxMotionDistance;
	particleSystemDesc.restParticleDistance = gFluidRestParticleDistance;
	particleSystemDesc.restOffset = gFluidRestOffset;
	particleSystemDesc.contactOffset = gFluidContactOffset;
	particleSystemDesc.gridSize = gFluidParticleRadius * 6;

	return new CFluidSystem(gPhysicsSDK, particleSystemDesc, MAX_FLUID_PARTICLES);
}

void ResetScene() {
	char buffer[255];
	sprintf_s(buffer, "Load/Reload scene: %s\n", gActiveFluidScenario ? gActiveFluidScenario->getName() : "No scene found.");
	printf(buffer);

	// Set scene properties
	if(gActiveFluidScenario)
		gScene->setGravity(gActiveFluidScenario->getGravity());

	// Release actors
	ClearScene();

	if(gActiveFluidScenario) {
		gFluidParticleRadius = gActiveFluidScenario->getParticleRadius();
		gFluidViscosity = gActiveFluidScenario->getViscosity();
		gFluidStiffness = gActiveFluidScenario->getStiffness();
		gFluidParticleDistance = gFluidParticleRadius * gActiveFluidScenario->getParticleDistanceFactor();
		gRigidBodyFallPos = gActiveFluidScenario->getActorCreatePosition();
		gFluidParticleRenderFactor = gActiveFluidScenario->getParticleRenderFactor();
	} else {
		gFluidParticleRadius = gActiveScene->getFluidParticleRadius();
		gFluidViscosity = gActiveScene->getFluidViscosity();
		gFluidStiffness = gActiveScene->getFluidStiffness();
		gFluidParticleDistance = gFluidParticleRadius * gActiveScene->getFluidParticleDistanceFactor();
		gRigidBodyFallPos = PxVec3(0.0f, 4.0f, 0.0f);
		gFluidParticleRenderFactor = gActiveScene->getFluidParticleRenderFactor();
	}

	// Add plane
	AddPlane();

	// Create fluid system
	gFluidSystem = CreateParticleFluidSystem();

	// Set GPU accelearion for particle fluid if supported
	gFluidSystem->setParticleBaseFlag(PxParticleBaseFlag::eCOLLISION_TWOWAY, true);

	if(gGPUDispatcher && gFluidUseGPUAcceleration)
		gFluidSystem->setParticleBaseFlag(PxParticleBaseFlag::eGPU, true);

	if(gFluidSystem) {
		PxActor *fluidActor = gFluidSystem->getActor();
		gScene->addActor(*fluidActor);
		gActors.push_back(fluidActor);
	}

	if(gActiveFluidScenario) {
		// Adding actors immediatly
		for(int i = 0; i < gActiveFluidScenario->getActors(); i++) {
			CActor *actor = gActiveFluidScenario->getActor(i);
			actor->setTimeElapsed(0.0f);

			if(actor->getTime() == -1) {
				AddScenarioActor(actor);
			}
		}

		// Adding waters immediatly
		for(int i = 0; i < gActiveFluidScenario->getFluidContainers(); i++) {
			FluidContainer *container = gActiveFluidScenario->getFluidContainer(i);
			container->TimeElapsed = 0.0f;
			container->EmitterElapsed = 0.0f;
			container->EmitterCoolDownElapsed = 0.0f;
			container->EmitterCoolDownActive = false;

			if(container->Time == -1 && !container->IsEmitter && gWaterAddBySceneChange) {
				AddWater(container, container->Type);
			}
		}
	}

	gTotalTimeElapsed = 0;

	// Simulate physx one time
	SingleStepPhysX(0.000001f);
}

void InitializePhysX() {
	cout << "  PhysX Version: " << PX_PHYSICS_VERSION_MAJOR << "." << PX_PHYSICS_VERSION_MINOR << "." << PX_PHYSICS_VERSION_BUGFIX << endl;
	cout << "  PhysX Foundation Version: " << PX_FOUNDATION_VERSION_MAJOR << "." << PX_FOUNDATION_VERSION_MINOR << "." << PX_FOUNDATION_VERSION_BUGFIX << endl;

	// Create Physics SDK
	gPhysXFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gPhysXFoundation, PxTolerancesScale());

	if(gPhysicsSDK == NULL) {
		cerr << "  Could not create PhysX SDK, exiting!" << endl;
		exit(1);
	}

	// Initialize PhysX Extensions
	if(!PxInitExtensions(*gPhysicsSDK, NULL)) {
		cerr << "  Could not initialize PhysX extensions, exiting!" << endl;
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
	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);

	// Default filter shader (No idea whats that about)
	sceneDesc.filterShader = gDefaultFilterShader;

	// CPU Dispatcher based on number of cpu cores
	int numThreads = gActiveScene->getNumCPUThreads();

	if(numThreads > COSLowLevel::getInstance()->getNumCPUCores())
		numThreads = COSLowLevel::getInstance()->getNumCPUCores();

	printf("  CPU acceleration supported (%d threads)\n", numThreads);
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(numThreads);

	// GPU Dispatcher
	gFluidUseGPUAcceleration = false;
	PxCudaContextManagerDesc cudaContextManagerDesc;
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
		cerr << "Could not create scene, exiting!" << endl;
		exit(1);
	}

	gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

	gDefaultMaterial = gPhysicsSDK->createMaterial(0.3f, 0.3f, 0.1f);
}

glm::mat4 getColumnMajor(PxMat33 m, PxVec3 t) {
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
			gFluidSystem->setExternalAcceleration(PxVec3(0.0f, 0.0f, 0.0f));
			gFluidLatestExternalAccelerationTime = -1;
		}
	}

	// Update PhysX
	if(!paused) {
		SingleStepPhysX(frametime);
	}
}

PxVec4 getColor(PxActor *actor, const PxVec4 &defaultColor) {
	if(actor->userData) {
		CActor *a = (CActor *)actor->userData;
		return a->getColor();
	} else {
		PxType actorType = actor->getConcreteType();
		if(actorType == PxConcreteType::eRIGID_STATIC)
			return DefaultStaticRigidBodyColor;
		else
			return defaultColor;
	}
}

void DrawBox(PxShape *pShape) {
	PxVec4 color = getColor(pShape->getActor(), DefaultDynamicRigidBodyCubeColor);

	PxTransform pT = PxShapeExt::getGlobalPose(*pShape, *pShape->getActor());
	PxBoxGeometry bg;
	pShape->getBoxGeometry(bg);
	PxMat33 m = PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 multm = gCamera.GetModelViewProjection() * mat;
	gRenderer->LoadMatrix(multm);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->getUniformLocation("color"), &color[0]);
	DrawGLCube(bg.halfExtents.x, bg.halfExtents.y, bg.halfExtents.z);
	gLightingShader->disable();
}

void DrawSphere(PxShape *pShape) {
	PxVec4 color = getColor(pShape->getActor(), DefaultDynamicRigidBodySphereColor);

	PxTransform pT = PxShapeExt::getGlobalPose(*pShape, *pShape->getActor());
	PxSphereGeometry sg;
	pShape->getSphereGeometry(sg);
	PxMat33 m = PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 multm = gCamera.GetModelViewProjection() * mat;
	gRenderer->LoadMatrix(multm);

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->getUniformLocation("color"), &color[0]);
	glutSolidSphere(sg.radius, 16, 16);
	gLightingShader->disable();
}

void DrawCapsule(PxShape *pShape) {
	PxVec4 color = getColor(pShape->getActor(), DefaultDynamicRigidBodyCapsuleColor);

	PxTransform pT = PxShapeExt::getGlobalPose(*pShape, *pShape->getActor());
	PxCapsuleGeometry cg;
	pShape->getCapsuleGeometry(cg);
	PxMat33 m = PxMat33(pT.q);
	glm::mat4 mat = getColumnMajor(m, pT.p);
	glm::mat4 multm = gCamera.GetModelViewProjection() * mat;

	gLightingShader->enable();
	gLightingShader->uniform4f(gLightingShader->getUniformLocation("color"), &color[0]);

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

void DrawShape(PxShape *shape) {
	PxGeometryType::Enum type = shape->getGeometryType();

	switch(type) {
		case PxGeometryType::eBOX:
			DrawBox(shape);
			break;

		case PxGeometryType::eSPHERE:
			DrawSphere(shape);
			break;

		case PxGeometryType::eCAPSULE:
			DrawCapsule(shape);
			break;
	}
}

void DrawBounds(const PxBounds3 &bounds) {
	PxVec3 center = bounds.getCenter();
	PxVec3 scale = bounds.getDimensions();

	GLfloat mat_diffuse[4] = { 0, 1, 1, 1 };
	glColor4fv(mat_diffuse);

	glm::mat4 m = gCamera.GetModelViewProjection();
	glm::mat4 mvp = glm::translate(m, glm::vec3(center.x, center.y, center.z));
	gRenderer->LoadMatrix(mvp);
	DrawGLCube(scale.x / 2, scale.y / 2, scale.z / 2);
}

void DrawActorBounding(PxActor *actor) {
	PxBounds3 bounds = actor->getWorldBounds();

	if(CFrustum::getInstance()->containsBounds(bounds)) {
		gDrawedActors++;
		DrawBounds(bounds);
	}
}

void DrawActor(PxActor *actor) {
	PxBounds3 bounds = actor->getWorldBounds();

	if(CFrustum::getInstance()->containsBounds(bounds)) {
		bool isVisible = true;
		bool blending = false;

		if(actor->userData) {
			CActor *a = (CActor *)actor->userData;
			isVisible = a->getVisible();
			blending = a->getBlending();
		}

		if(isVisible) {
			gDrawedActors++;

			PxType actorType = actor->getConcreteType();
			if(actorType == PxConcreteType::eRIGID_STATIC || actorType == PxConcreteType::eRIGID_DYNAMIC) {
				PxRigidActor *rigActor = (PxRigidActor *)actor;
				PxU32 nShapes = rigActor->getNbShapes();
				PxShape **shapes = new PxShape * [nShapes];

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
	for(long index = 0; index < (long)gActors.size(); ++index) {
		PxActor *act = gActors.at(index);
		DrawActor(act);
	}
}

void RenderActorBoundings() {
	// Render all the actors in the scene as bounding volume
	for(long index = 0; index < (long)gActors.size(); ++index) {
		PxActor *act = gActors.at(index);
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
		if (gPhysXVisualDebugger->isConnected())
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

const char *GetGeometryType(unsigned int state) {
	switch(state) {
		case 1:
			return "Box\0";

		case 2:
			return "Sphere\0";

		case 3:
			return "Capsule\0";

		case 4:
			return "Fluid - single\0";

		case 5:
			return "Fluid - plane\0";

		case 6:
			return "Fluid - cube\0";

		case 7:
			return "Fluid - sphere\0";

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

const char *GetFluidRenderMode(unsigned int mode) {
	switch(mode) {
		case SSFRenderMode_Disabled:
			return "Disabled\0";

		case SSFRenderMode_Fluid:
			return "Fluid\0";

		case SSFRenderMode_PointSprites:
			return "Point Sprites\0";

		case SSFRenderMode_Points:
			return "Points\0";

		default:
			return "None\0";
	}
}

const char *GetFluidDebugType(unsigned int type) {
	switch(type) {
		case SWOWTYPE_FINAL:
			return "Final\0";

		case SWOWTYPE_DEPTH:
			return "Depth only\0";

		case SWOWTYPE_NORMAL:
			return "Normal only\0";

		case SWOWTYPE_COLOR:
			return "Color only\0";

		case SWOWTYPE_DIFFUSE:
			return "Diffuse only\0";

		case SWOWTYPE_SPECULAR:
			return "Specular only\0";

		case SWOWTYPE_DIFFUSE_SPECULAR:
			return "Diffuse + Specular\0";

		case SWOWTYPE_SCENE:
			return "Scene only\0";

		case SWOWTYPE_FRESNEL:
			return "Fresne onlyl\0";

		case SWOWTYPE_REFRACTION:
			return "Refraction only\0";

		case SWOWTYPE_REFLECTION:
			return "Reflection only\0";

		case SWOWTYPE_FRESNEL_REFLECTION:
			return "Fresnel + Reflection\0";

		case SWOWTYPE_THICKNESS:
			return "Thickness only\0";

		case SWOWTYPE_ABSORBTION:
			return "Color absorbition only\0";

		default:
			return "Unknown\0";
	}
}

void CreateActorsBasedOnTime(const float frametime) {
	// Add not fallen fluids from active scenario
	if(gActiveFluidScenario) {
		// Add actors
		for(int i = 0; i < gActiveFluidScenario->getActors(); i++) {
			CActor *actor = gActiveFluidScenario->getActor(i);

			if(actor->getTime() > 0) {
				if(actor->getTimeElapsed() < (float)actor->getTime()) {
					actor->setTimeElapsed(actor->getTimeElapsed() + frametime);

					if(actor->getTimeElapsed() >= (float)actor->getTime()) {
						AddScenarioActor(actor);
					}
				}
			}
		}

		// Add fluids
		for(int i = 0; i < gActiveFluidScenario->getFluidContainers(); i++) {
			FluidContainer *container = gActiveFluidScenario->getFluidContainer(i);

			float time;

			if(!container->IsEmitter) {
				// Einmaliger partikel emitter
				if(container->Time > 0) {
					time = (float)container->Time;

					if(container->TimeElapsed < time) {
						container->TimeElapsed += frametime;

						if(container->TimeElapsed >= time) {
							AddWater(container, container->Type);
						}
					}
				}
			} else if(!gStoppedEmitter) {
				time = container->EmitterTime;
				float duration = (float)container->EmitterDuration;

				if(time > 0.0f) {
					container->EmitterElapsed += frametime;

					if((container->EmitterElapsed < duration) || (container->EmitterDuration == 0)) {
						if(container->TimeElapsed < time) {
							container->TimeElapsed += frametime;

							if(container->TimeElapsed >= time) {
								container->TimeElapsed = 0.0f;
								AddWater(container, container->Type);
							}
						}
					} else if(container->EmitterCoolDown > 0.0f) {
						if(!container->EmitterCoolDownActive) {
							container->EmitterCoolDownActive = true;
							container->EmitterCoolDownElapsed = 0.0f;
						}

						if(container->EmitterCoolDownActive) {
							container->EmitterCoolDownElapsed += frametime;

							if(container->EmitterCoolDownElapsed >= (float)container->EmitterCoolDown) {
								// Cool down finished
								container->EmitterCoolDownActive = false;
								container->EmitterElapsed = 0.0f;
								container->TimeElapsed = 0.0f;
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

string drawingError = "";
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
		sprintf_s(buffer, "Drawed actors: %d of %d", gDrawedActors, gTotalActors);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Total fluid particles: %d", gTotalFluidParticles);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw error: %s", drawingError.c_str());
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Simulation state (O): %s", paused ? "paused" : "running");
		RenderOSDLine(osdPos, buffer);

		// Empty line
		osdPos.newLine();

		FluidColor *activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);

		sprintf_s(buffer, "Controls:");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Geometry type (1-7): %s", GetGeometryType(gGeoType));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Wireframe (W): %s", gDrawWireframe ? "enabled" : "disabled");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Boundbox (B): %s", gDrawBoundBox ? "enabled" : "disabled");
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Draw Rigidbodies (D): %s", GetDrawRigidbodyStr(gHideRigidBodies));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid Rendering Mode (S): %s", GetFluidRenderMode(gSSFRenderMode));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid color (C): %d / %d - %s", gSSFCurrentFluidIndex + 1, gActiveScene->getFluidColors(), activeFluidColor->name.c_str());
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
		sprintf_s(buffer, "    Fluid debug type: %d / %d (%s)", gFluidDebugType, MAX_DEBUGTYPE, GetFluidDebugType(gFluidDebugType));
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid color falloff scale: %f", activeFluidColor->falloffScale);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "    Fluid color falloff alpha: %f", activeFluidColor->falloff.w);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid scenario (L): %d / %d - %s", gActiveFluidScenarioIdx + 1, gFluidScenarios.size(), gActiveFluidScenario ? gActiveFluidScenario->getName() : "No scenario loaded!");
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

		sprintf_s(buffer, "Fluid particle radius/distance: %f / %f", gFluidParticleRadius, gFluidParticleDistance);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid rest particle distance: %f", gFluidRestParticleDistance);
		RenderOSDLine(osdPos, buffer);
		sprintf_s(buffer, "Fluid min density: %f", gActiveFluidScenario ? gActiveFluidScenario->getParticleMinDensity() : gActiveScene->getFluidParticleMinDensity());
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
	gSkyboxShader->uniformMatrix4(gSkyboxShader->getUniformLocation("mvp"), &mvp[0][0]);
	gSkyboxShader->uniform1i(gSkyboxShader->getUniformLocation("cubemap"), 0);

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
	FluidColor *activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);;
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
	glm::mat4 mvp = gCamera.GetModelViewProjection();
	glm::mat4 proj = gCamera.GetProjection();
	glm::mat4 mdlv = gCamera.GetModelview();
	gRenderer->LoadMatrix(mvp);

	// Update (Frustum and PhysX)
	Update(proj, mdlv, 1.0f / 60.0f);

	// Clear back buffer
	PxVec3 backcolor = gActiveScene->getBackgroundColor();
	gRenderer->ClearColor(backcolor.x, backcolor.y, backcolor.z, 0.0f);
	gRenderer->Clear(ClearFlags::Color | ClearFlags::Depth);

	bool drawFluidParticles = gSSFRenderMode != SSFRenderMode_Disabled;

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

void AddActorByState(unsigned int state) {
	switch(state) {
		case 1:
			AddBox(PxVec3(0.5, 0.5, 0.5), NULL);
			break;

		case 2:
			AddSphere(NULL);
			break;

		case 3:
			AddCapsule();
			break;

		case 4:
			AddWater(FluidTypeDrop);
			break;

		case 5:
			AddWater(FluidTypeWall);
			break;

		case 6:
			AddWater(FluidTypeBlob);
			break;

		case 7:
			AddWater(FluidTypeSphere);
			break;

		default:
			break;
	}
}

void ToggleFluidGPUAcceleration() {
	if(gGPUDispatcher) {
		PxActor *fluidActor = gFluidSystem->getActor();
		gFluidUseGPUAcceleration = !gFluidUseGPUAcceleration;
		gScene->removeActor(*fluidActor);
		gFluidSystem->setParticleBaseFlag(PxParticleBaseFlag::eGPU, gFluidUseGPUAcceleration);
		gScene->addActor(*fluidActor);
	}
}

void SetFluidExternalAcceleration(const PxVec3 &acc) {
	gFluidLatestExternalAccelerationTime = glutGet(GLUT_ELAPSED_TIME) + 3000; // 3 Seconds
	gFluidSystem->setExternalAcceleration(acc);
}

void KeyDownSpecial(int key, int x, int y) {
	const float accSpeed = 10.0f;
	const PxForceMode::Enum accMode = PxForceMode::eACCELERATION;

	switch(key) {
		case GLUT_KEY_RIGHT:
		{
			gFluidSystem->addForce(PxVec3(1.0f * accSpeed, 0.0f, 0.0f), accMode);
			break;
		}

		case GLUT_KEY_LEFT:
		{
			gFluidSystem->addForce(PxVec3(-1.0f * accSpeed, 0.0f, 0.0f), accMode);
			break;
		}

		case GLUT_KEY_UP:
		{
			gFluidSystem->addForce(PxVec3(0.0f, 0.0f, -1.0f * accSpeed), accMode);
			break;
		}

		case GLUT_KEY_DOWN:
		{
			gFluidSystem->addForce(PxVec3(0.0f, 0.0f, 1.0f * accSpeed), accMode);
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
			gGeoType = 1 + (key - 49);
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
			gFluidDebugType = 0;
			gSSFRenderMode++;

			if(gSSFRenderMode > SSFRenderMode_Disabled) gSSFRenderMode = SSFRenderMode_Fluid;

			if(gSSFRenderMode == SSFRenderMode_Fluid && !gFluidRenderer->IsSupported())
				gSSFRenderMode = SSFRenderMode_PointSprites;

			SingleStepPhysX(0.000001f);
			break;
		}

		case 99: // c
		{
			gSSFCurrentFluidIndex++;

			if(gSSFCurrentFluidIndex > gActiveScene->getFluidColors() - 1) gSSFCurrentFluidIndex = 0;

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
			gActiveFluidScenario->setViscosity(gFluidViscosity);
			break;
		}

		case FLUID_PROPERTY_STIFFNESS:
		{
			gFluidStiffness += value;
			gFluidSystem->setStiffness(gFluidStiffness);
			gActiveFluidScenario->setStiffness(gFluidStiffness);
			break;
		}

		case FLUID_PROPERTY_MAXMOTIONDISTANCE:
		{
			gFluidMaxMotionDistance += value / 1000.0f;
			gFluidSystem->setMaxMotionDistance(gFluidMaxMotionDistance);
			gActiveScene->setFluidMaxMotionDistance(gFluidMaxMotionDistance);
			break;
		}

		case FLUID_PROPERTY_CONTACTOFFSET:
		{
			gFluidContactOffset += value / 1000.0f;
			gFluidSystem->setContactOffset(gFluidContactOffset);
			gActiveScene->setFluidContactOffset(gFluidContactOffset);
			break;
		}

		case FLUID_PROPERTY_RESTOFFSET:
		{
			gFluidRestOffset += value / 1000.0f;
			gFluidSystem->setRestOffset(gFluidRestOffset);
			gActiveScene->setFluidRestOffset(gFluidRestOffset);
			break;
		}

		case FLUID_PROPERTY_RESTITUTION:
		{
			gFluidRestitution += value / 1000.0f;
			gFluidSystem->setRestitution(gFluidRestitution);
			gActiveScene->setFluidRestitution(gFluidRestitution);
			break;
		}

		case FLUID_PROPERTY_DAMPING:
		{
			gFluidDamping += value / 1000.0f;
			gFluidSystem->setDamping(gFluidDamping);
			gActiveScene->setFluidDamping(gFluidDamping);
			break;
		}

		case FLUID_PROPERTY_DYNAMICFRICTION:
		{
			gFluidDynamicFriction += value / 1000.0f;
			gFluidSystem->setDynamicFriction(gFluidDynamicFriction);
			gActiveScene->setFluidDynamicFriction(gFluidDynamicFriction);
			break;
		}

		case FLUID_PROPERTY_PARTICLEMASS:
		{
			gFluidParticleMass += value / 1000.0f;
			gFluidSystem->setParticleMass(gFluidParticleMass);
			gActiveScene->setFluidParticleMass(gFluidParticleMass);
			break;
		}

		case FLUID_PROPERTY_DEPTH_BLUR_SCALE:
		{
			gSSFBlurDepthScale += value / 10000.0f;
			break;
		}

		case FLUID_PROPERTY_PARTICLE_RENDER_FACTOR:
		{
			gFluidParticleRenderFactor += value / 10.0f;
			gFluidParticleRenderFactor = roundFloat(gFluidParticleRenderFactor);
			break;
		}

		case FLUID_PROPERTY_DEBUGTYPE:
		{
			int inc = (int)value;
			gFluidDebugType += inc;

			if(gFluidDebugType < 0) gFluidDebugType = MAX_DEBUGTYPE;

			if(gFluidDebugType > MAX_DEBUGTYPE) gFluidDebugType = SWOWTYPE_FINAL;

			break;
		}

		case FLUID_PROPERTY_COLOR_FALLOFF_SCALE:
		{
			FluidColor *activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);
			activeFluidColor->falloffScale += value / 100.0f;
			break;
		}

		case FLUID_PROPERTY_COLOR_FALLOFF_ALPHA:
		{
			FluidColor *activeFluidColor = gActiveScene->getFluidColor(gSSFCurrentFluidIndex);
			activeFluidColor->falloff.w += value / 100.0f;
			break;
		}
	}
}

void KeyDown(unsigned char key, int x, int y) {
	switch(key) {
		case 32: // Space
		{
			AddActorByState(gGeoType);
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
	vector<string> scenFiles = COSLowLevel::getInstance()->getFilesInDirectory("scenarios\\*.xml");

	for(unsigned int i = 0; i < scenFiles.size(); i++) {
		string filename = "scenarios\\";
		filename += scenFiles[i];
		CFluidScenario *scenario = CFluidScenario::load(filename.c_str(), gActiveScene);
		gFluidScenarios.push_back(scenario);
	}

	if(gFluidScenarios.size() > 0) {
		gActiveFluidScenarioIdx = 0;
		gActiveFluidScenario = gFluidScenarios[gActiveFluidScenarioIdx];
	} else {
		gActiveFluidScenarioIdx = -1;
		gActiveFluidScenario = NULL;
		cerr << "  No fluid scenario found, fluid cannot be used!" << endl;
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
	gActiveScene = new CScene(DefaultFluidViscosity, DefaultFluidStiffness, DefaultFluidParticleDistanceFactor, DefaultFluidParticleRenderFactor, DefaultFluidParticleRadius, DefaultFluidParticleMinDensity, gDefaultRigidBodyDensity);
	gActiveScene->load("scene.xml");
	gFluidParticleRadius = gActiveScene->getFluidParticleRadius();
	gFluidParticleRenderFactor = gActiveScene->getFluidParticleRenderFactor();
	gSSFCurrentFluidIndex = gActiveScene->getFluidColorDefaultIndex();

	// Create spherical point sprites
	printf("  Allocate spherical point sprites\n");
	gPointSprites = new CSphericalPointSprites();
	gPointSprites->Allocate(MAX_FLUID_PARTICLES);

	// Create spherical point sprites shader
	printf("  Load spherical point sprites shader\n");
	gPointSpritesShader = new CGLSL();
	Utils::attachShaderFromFile(gPointSpritesShader, GL_VERTEX_SHADER, "shaders\\PointSprites.vertex", "    ");
	Utils::attachShaderFromFile(gPointSpritesShader, GL_FRAGMENT_SHADER, "shaders\\PointSprites.fragment", "    ");

	// Create scene FBO
	printf("  Create scene FBO\n");
	gSceneFBO = new CFBO(windowWidth, windowHeight);
	gSceneFBO->addRenderTarget(GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, GL_DEPTH_ATTACHMENT, TEXTURE_ID_SCENE_DEPTH, GL_NEAREST);
	gSceneFBO->addTextureTarget(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_COLOR_ATTACHMENT0, TEXTURE_ID_SCENE_SCENE, GL_LINEAR);
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
	gFluidRenderer->SetSceneTexture(gSceneFBO->getTexture(TEXTURE_ID_SCENE_SCENE));
	gFluidRenderer->SetSkyboxCubemap(gSkyboxCubemap);

	if(gFluidRenderer->IsSupported())
		gSSFRenderMode = SSFRenderMode_Fluid;
	else
		gSSFRenderMode = SSFRenderMode_PointSprites;

	// Create lightung shader
	printf("  Create lighting renderer\n");
	gLightingShader = new CGLSL();
	Utils::attachShaderFromFile(gLightingShader, GL_VERTEX_SHADER, "shaders\\Lighting.vertex", "    ");
	Utils::attachShaderFromFile(gLightingShader, GL_FRAGMENT_SHADER, "shaders\\Lighting.fragment", "    ");

	// Create skybox vbo and shader
	printf("  Create skybox\n");
	gSkyboxVBO = Primitives::createCube(100, 100, 100, false);
	gSkyboxShader = new CGLSL();
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
		CFluidScenario *scen = gFluidScenarios[i];
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
		cerr << "Could not initialize glew!" << endl;
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
		cerr << endl << "Your graphics adapter is not supported, press any key to exit!" << endl;
		cerr << "Required opengl version:" << endl;
		cerr << "  OpenGL version 2.0 or higher" << endl;
		cerr << "Required opengl extensions:" << endl;
		cerr << "  GL_ARB_texture_float" << endl;
		cerr << "  GL_ARB_point_sprite" << endl;
		cerr << "  GL_ARB_framebuffer_object" << endl;
		cerr << "Required constants:" << endl;
		cerr << "  GL_MAX_COLOR_ATTACHMENTS >= 4" << endl;
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
