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

#include <rapidxml/rapidxml.hpp>

#include "OSLowLevel.h"
#include "XMLUtils.h"
#include "Utils.h"
#include "AllActors.hpp"
#include "VariableManager.h"

Scenario::Scenario() {
	this->name[0] = 0;
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

Scenario *Scenario::load(const char *filename, CScene *scene) {
	if(COSLowLevel::fileExists(filename)) {
		std::cout << "  Load scenario from file '" << filename << "'" << std::endl;

		std::string xml = COSLowLevel::getTextFileContent(filename);
		std::vector<char> xml_copy = Utils::toCharVector(xml);

		// First get variables
		rapidxml::xml_document<> doc;
		try {
			doc.parse<0>(&xml_copy[0]);
		} catch(...) {
			std::cerr << "could not parse xml!" << std::endl;
			return nullptr;
		}
		rapidxml::xml_node<> *rootNode = doc.first_node("Scenario");
		if(!rootNode) {
			doc.clear();
			return(nullptr);
		}

		VariableManager varMng = VariableManager();

		rapidxml::xml_node<> *varsNode = rootNode->first_node("Variables");
		if(varsNode) {
			varMng.Parse(varsNode);
		}

		XMLUtils xmlUtils = XMLUtils(&varMng);

		Scenario *newScenario = new Scenario();

		// Name
		rapidxml::xml_node<> *nameNode = rootNode->first_node("Name");
		if(nameNode) {
			strcpy_s(newScenario->name, sizeof(newScenario->name), nameNode->value());
		}

		// Gravity
		newScenario->gravity = xmlUtils.getNodeVec3(rootNode, "Gravity", glm::vec3(0.0f, -9.8f, 0.0f));

		// Fluid properties
		rapidxml::xml_node<> *fpNode = rootNode->first_node("FluidProperties");
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
		rapidxml::xml_node<> *apNode = rootNode->first_node("ActorProperties");
		if(apNode) {
			newScenario->actorCreatePosition = xmlUtils.getNodeVec3(apNode, "CreatePosition", glm::vec3(0.0f));
		}

		// Actors
		rapidxml::xml_node<> *actorsNode = rootNode->first_node("Actors");
		if(actorsNode) {
			std::vector<rapidxml::xml_node<> *> actors = xmlUtils.getChilds(actorsNode, "Actor");
			std::vector<rapidxml::xml_node<> *>::iterator p;
			for(p = actors.begin(); p != actors.end(); ++p) {
				rapidxml::xml_node<> *actorNode = *p;

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
		rapidxml::xml_node<> *fluidsNode = rootNode->first_node("Fluids");
		if(fluidsNode) {
			std::vector<rapidxml::xml_node<> *> fluids = xmlUtils.getChilds(fluidsNode, "Fluid");
			std::vector<rapidxml::xml_node<> *>::iterator p;
			for(p = fluids.begin(); p != fluids.end(); ++p) {
				rapidxml::xml_node<> *fluidNode = *p;
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

		return newScenario;
	} else {
		return nullptr;
	}
}