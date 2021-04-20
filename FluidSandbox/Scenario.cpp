/*
======================================================================================================================
	Fluid Sandbox - Scenario.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "Scenario.h"

#include <iostream>

#include <glm/glm.hpp>

#include <final_xml.h>

#include "OSLowLevel.h"
#include "XMLUtils.h"
#include "Utils.h"
#include "AllActors.hpp"
#include "VariableManager.h"

Scenario::Scenario() {
	this->fileName[0] = 0;
	this->displayName[0] = 0;
	this->actorCreatePosition = glm::vec3(0.0f, 0.0f, 0.0f);
	this->sim = FluidSimulationProperties();
	this->render = FluidRenderProperties();
	this->gravity = glm::vec3(0.0f, -9.8f, 0.0f);
}


Scenario::~Scenario(void) {
	for(size_t i = 0; i < fluids.size(); i++) {
		const FluidActor *fluid = fluids[i];
		delete fluid;
	}
	fluids.clear();

	for(size_t i = 0; i < bodies.size(); i++) {
		const Actor *body = bodies[i];
		delete body;
	}
	bodies.clear();
}

Scenario *Scenario::load(const char *filePath, CScene *scene) {
	if(COSLowLevel::fileExists(filePath)) {
		std::cout << "  Load scenario from file '" << filePath << "'" << std::endl;

		// Load file content
		std::string xml = COSLowLevel::getTextFileContent(filePath);

		// Parse XML
		fxmlContext ctx = FXML_ZERO_INIT;
		fxmlTag root = FXML_ZERO_INIT;
		size_t xmlLen = xml.length();
		if(!fxmlInitFromMemory(xml.c_str(), xmlLen, &ctx)) {
			std::cerr << "Failed initialize XML context with xml length of " << xml << std::endl;
			return nullptr;
		}
		if(!fxmlParse(&ctx, &root)) {
			std::cerr << "Failed to parse XML file '" << filePath << "'!" << std::endl;
			return(nullptr);
		}

		const fxmlTag *rootNode = fxmlFindTagByName(&root, "Scenario");
		if(!rootNode) {
			fxmlFree(&ctx);
			return(nullptr);
		}

		VariableManager varMng = VariableManager();

		const fxmlTag *varsNode = fxmlFindTagByName(rootNode, "Variables");
		if(varsNode) {
			varMng.Parse(varsNode);
		}

		XMLUtils xmlUtils = XMLUtils(&varMng);

		Scenario *newScenario = new Scenario();

		// Name
		const fxmlTag *nameNode = fxmlFindTagByName(rootNode, "Name");
		if(nameNode) {
			strcpy_s(newScenario->displayName, sizeof(newScenario->displayName), nameNode->value);
		}

		// Gravity
		newScenario->gravity = xmlUtils.getNodeVec3(rootNode, "Gravity", glm::vec3(0.0f, -9.8f, 0.0f));

		// Fluid properties
		const fxmlTag *fpNode = fxmlFindTagByName(rootNode, "FluidProperties");
		if(fpNode) {
			float particleRadius = xmlUtils.getNodeFloat(fpNode, "ParticleRadius", scene->sim.particleRadius);
			float particleDistanceFactor = xmlUtils.getNodeFloat(fpNode, "ParticleDistanceFactor", scene->sim.particleDistanceFactor);

			newScenario->sim = FluidSimulationProperties::Compute(particleRadius, particleDistanceFactor);
			newScenario->render = scene->render;

			newScenario->sim.viscosity = xmlUtils.getNodeFloat(fpNode, "Viscosity", scene->sim.viscosity);
			newScenario->sim.stiffness = xmlUtils.getNodeFloat(fpNode, "Stiffness", scene->sim.stiffness);
			newScenario->sim.restitution = xmlUtils.getNodeFloat(fpNode, "Restitution", scene->sim.restitution);
			newScenario->sim.damping = xmlUtils.getNodeFloat(fpNode, "Damping", scene->sim.damping);
			newScenario->sim.dynamicFriction = xmlUtils.getNodeFloat(fpNode, "DynamicFriction", scene->sim.dynamicFriction);
			newScenario->sim.maxMotionDistance = xmlUtils.getNodeFloat(fpNode, "MaxMotionDistance", scene->sim.maxMotionDistance);
			newScenario->sim.restOffset = xmlUtils.getNodeFloat(fpNode, "RestOffset", scene->sim.restOffset);
			newScenario->sim.contactOffset = xmlUtils.getNodeFloat(fpNode, "ContactOffset", scene->sim.contactOffset);
			newScenario->sim.particleMass = xmlUtils.getNodeFloat(fpNode, "ParticleMass", scene->sim.particleMass);

			newScenario->render.particleRenderFactor = xmlUtils.getNodeFloat(fpNode, "ParticleRenderFactor", scene->render.particleRenderFactor);
			newScenario->render.minDensity = xmlUtils.getNodeFloat(fpNode, "ParticleMinDensity", scene->render.minDensity);
		} else {
			newScenario->sim = scene->sim;
			newScenario->render = scene->render;
		}

		// Actor properties
		const fxmlTag *apNode = fxmlFindTagByName(rootNode, "ActorProperties");
		if(apNode) {
			newScenario->actorCreatePosition = xmlUtils.getNodeVec3(apNode, "CreatePosition", glm::vec3(0.0f));
		}

		// Actors
		const fxmlTag *actorsNode = fxmlFindTagByName(rootNode, "Actors");
		if(actorsNode) {
			std::vector<const fxmlTag *> actors = xmlUtils.getChilds(actorsNode, "Actor");
			for(auto p = actors.begin(); p != actors.end(); ++p) {
				const fxmlTag *actorNode = *p;

				std::string type = xmlUtils.getAttribute(actorNode, "type", "");
				ActorMovementType atype = Utils::toActorMovementType(type.c_str());

				std::string primitive = xmlUtils.getAttribute(actorNode, "primitive", "");

				glm::vec3 pos = xmlUtils.getAttributeVec3(actorNode, "pos", glm::vec3(0.0f));

				glm::vec3 extents = xmlUtils.getAttributeVec3(actorNode, "extents", glm::vec3(0));
				if(glm::length(extents) == 0) {
					glm::vec3 size = xmlUtils.getAttributeVec3(actorNode, "size", glm::vec3(0));
					extents = size * 0.5f;
				}

				glm::vec4 color = xmlUtils.getAttributeVec4(actorNode, "color", glm::vec4(1, 1, 1, 1));
				glm::vec3 velocity = xmlUtils.getAttributeVec3(actorNode, "vel", glm::vec3(0.0f));

				// TODO(final): Change scenario format to degrees instead and convert it to radians here!
				glm::vec3 eulerRotation = xmlUtils.getAttributeVec3(actorNode, "rot", glm::vec3(0.0f));

				bool defaultBlending = (atype == ActorMovementType::Static);

				int actorTime = xmlUtils.getAttributeS32(actorNode, "time", 0);
				float density = xmlUtils.getAttributeFloat(actorNode, "density", scene->defaultActorDensity);
				float radius = xmlUtils.getAttributeFloat(actorNode, "radius", 0.5f);
				float halfHeight = xmlUtils.getAttributeFloat(actorNode, "halfHeight", 0.5f);
				bool visible = xmlUtils.getAttributeBool(actorNode, "visible", true);
				bool blending = xmlUtils.getAttributeBool(actorNode, "blending", defaultBlending);
				bool particleDrain = xmlUtils.getAttributeBool(actorNode, "particleDrain", false);

				Actor *newBody = nullptr;
				if(strcmp(primitive.c_str(), "cube") == 0) {
					newBody = new CubeActor(atype, extents);
				} else if(strcmp(primitive.c_str(), "sphere") == 0) {
					newBody = new SphereActor(atype, radius);
				} else if(strcmp(primitive.c_str(), "capsule") == 0) {
					newBody = new CapsuleActor(atype, radius, halfHeight);
				} else {
					std::cerr << "    Actor primitive type '" << primitive << "' is not valid!" << std::endl;
				}

				if(newBody != nullptr) {
					newBody->transform.position = pos;
					newBody->transform.rotation = glm::quat(eulerRotation);
					newBody->time = actorTime;
					newBody->color = color;
					newBody->density = density;
					newBody->velocity = velocity;
					newBody->visible = visible;
					newBody->blending = blending;
					newBody->particleDrain = particleDrain;
					newBody->isTemplate = true;
					newScenario->bodies.push_back(newBody);
				}

			}
		}

		// Fluids
		const fxmlTag *fluidsNode = fxmlFindTagByName(rootNode, "Fluids");
		if(fluidsNode) {
			std::vector<const fxmlTag *> fluids = xmlUtils.getChilds(fluidsNode, "Fluid");
			for(auto p = fluids.begin(); p != fluids.end(); ++p) {
				const fxmlTag *fluidNode = *p;
				std::string fluidTypeStr = xmlUtils.getAttribute(fluidNode, "type", "blob");
				FluidType fluidType = Utils::toFluidType(fluidTypeStr.c_str());
				glm::vec3 pos = xmlUtils.getAttributeVec3(fluidNode, "pos", glm::vec3(0.0f));
				glm::vec3 size = xmlUtils.getAttributeVec3(fluidNode, "size", glm::vec3(0.0f));
				glm::vec3 velocity = xmlUtils.getAttributeVec3(fluidNode, "vel", glm::vec3(0.0f));
				int fluidTime = xmlUtils.getAttributeS32(fluidNode, "time", 0);
				float radius = xmlUtils.getAttributeFloat(fluidNode, "radius", 0.0);
				bool isEmitter = xmlUtils.getAttributeBool(fluidNode, "isEmitter", false);
				float emitterRate = xmlUtils.getAttributeFloat(fluidNode, "emitterRate", 0.0);
				float emitterTime = emitterRate > 0.0f ? 1000.0f / emitterRate : 0.0f;
				uint32_t emitterDuration = xmlUtils.getAttributeU32(fluidNode, "emitterDuration", 0);
				uint32_t emitterCoolDown = xmlUtils.getAttributeU32(fluidNode, "emitterCoolDown", 0);
				FluidActor *fluidCon = new FluidActor(size, radius, fluidType);
				fluidCon->transform.position = pos;
				fluidCon->velocity = velocity;
				fluidCon->time = fluidTime;
				fluidCon->radius = radius;
				fluidCon->isEmitter = isEmitter;
				fluidCon->emitterRate = emitterRate;
				fluidCon->emitterTime = emitterTime;
				fluidCon->emitterDuration = emitterDuration;
				fluidCon->emitterCoolDown = emitterCoolDown;
				fluidCon->isTemplate = true;
				newScenario->fluids.push_back(fluidCon);
			}
		}

		fxmlFree(&ctx);

		return newScenario;
	} else {
		return nullptr;
	}
}