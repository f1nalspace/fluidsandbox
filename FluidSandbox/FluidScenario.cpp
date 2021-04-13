#include "FluidScenario.h"

#include <iostream>

#include <glm/glm.hpp>

#include <rapidxml/rapidxml.hpp>

#include "OSLowLevel.h"
#include "XMLUtils.h"
#include "Utils.h"
#include "CubeActor.hpp"
#include "SphereActor.hpp"

CFluidScenario::CFluidScenario() {
	this->name[0] = 0;
	this->actorCreatePosition = physx::PxVec3(0.0f, 0.0f, 0.0f);
	this->viscosity = 20.0f;
	this->stiffness = 35.0f;
	this->damping = 0.0f;
	this->particleDistanceFactor = 2.0f;
	this->particleRenderFactor = 1.0f;
	this->particleRadius = 0.05f;
	this->particleMinDensity = 0.01f;
	this->gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
}


CFluidScenario::~CFluidScenario(void) {
	for(size_t i = 0; i < fluidContainers.size(); i++) {
		FluidContainer *container = fluidContainers[i];
		delete container;
	}
	fluidContainers.clear();

	for(size_t i = 0; i < actors.size(); i++) {
		CActor *actor = actors[i];
		delete actor;
	}
	actors.clear();
}

CFluidScenario *CFluidScenario::load(const char *filename, CScene *scene) {
	if(COSLowLevel::getInstance()->fileExists(filename)) {
		std::cout << "  Load scenario from file '" << filename << "'" << std::endl;

		std::string xml = COSLowLevel::getInstance()->getTextFileContent(filename);
		std::vector<char> xml_copy = Utils::toCharVector(xml);

		std::map<std::string, std::string> variables;

		// First get variables
		rapidxml::xml_document<> doc;
		try {
			doc.parse<0>(&xml_copy[0]);
		} catch(...) {
			std::cerr << "could not parse xml!" << std::endl;
			return NULL;
		}
		rapidxml::xml_node<> *rootNode = doc.first_node("Scenario");
		if(rootNode) {
			rapidxml::xml_node<> *varsNode = rootNode->first_node("Variables");
			if(varsNode) {
				std::vector<rapidxml::xml_node<> *> childs = XMLUtils::getChilds(varsNode);
				std::vector<rapidxml::xml_node<> *>::iterator p;
				for(p = childs.begin(); p != childs.end(); ++p) {
					rapidxml::xml_node<> *varNode = *p;
					variables.insert(std::pair<std::string, std::string>(varNode->name(), varNode->value()));
				}
			}
		}

		// Now replace all strings in xml content
		std::map<std::string, std::string>::iterator p;
		for(p = variables.begin(); p != variables.end(); ++p) {
			std::string name = p->first;
			name = "{%" + name + "}";
			std::string value = p->second;
			Utils::replaceString(xml, name, value);
		}

		// Now reload xml again
		doc.clear();
		xml_copy = Utils::toCharVector(xml);
		doc.parse<0>(&xml_copy[0]);

		CFluidScenario *newScenario = new CFluidScenario();

		rootNode = doc.first_node("Scenario");
		if(rootNode) {
			// Name
			rapidxml::xml_node<> *nameNode = rootNode->first_node("Name");
			if(nameNode) {
				strcpy_s(newScenario->name, sizeof(newScenario->name), nameNode->value());
			}

			// Gravity
			rapidxml::xml_node<> *gravitiyNode = rootNode->first_node("Gravity");
			if(gravitiyNode) {
				newScenario->gravity = Utils::toVec3(gravitiyNode->value(), physx::PxVec3(0.0f, -9.8f, 0.0f));
			}

			// Fluid properties
			rapidxml::xml_node<> *fpNode = rootNode->first_node("FluidProperties");
			if(fpNode) {
				newScenario->viscosity = XMLUtils::findNodeFloat(fpNode, "Viscosity", scene->fluidViscosity);
				newScenario->stiffness = XMLUtils::findNodeFloat(fpNode, "Stiffness", scene->fluidStiffness);
				newScenario->damping = XMLUtils::findNodeFloat(fpNode, "Damping", scene->fluidDamping);
				newScenario->particleDistanceFactor = XMLUtils::findNodeFloat(fpNode, "ParticleDistanceFactor", scene->fluidParticleDistanceFactor);
				newScenario->particleRenderFactor = XMLUtils::findNodeFloat(fpNode, "ParticleRenderFactor", scene->fluidParticleRenderFactor);
				newScenario->particleRadius = XMLUtils::findNodeFloat(fpNode, "ParticleRadius", scene->fluidParticleRadius);
				newScenario->particleMinDensity = XMLUtils::findNodeFloat(fpNode, "ParticleMinDensity", scene->fluidParticleMinDensity);
			} else {
				newScenario->viscosity = scene->fluidViscosity;
				newScenario->stiffness = scene->fluidStiffness;
				newScenario->damping = scene->fluidDamping;
				newScenario->particleDistanceFactor = scene->fluidParticleDistanceFactor;
				newScenario->particleRenderFactor = scene->fluidParticleRenderFactor;
				newScenario->particleRadius = scene->fluidParticleRadius;
				newScenario->particleMinDensity = scene->fluidParticleMinDensity;
			}

			// Actor properties
			rapidxml::xml_node<> *apNode = rootNode->first_node("ActorProperties");
			if(apNode) {
				newScenario->actorCreatePosition = Utils::toVec3(XMLUtils::findNodeValue(apNode, "CreatePosition", "0, 0, 0"), physx::PxVec3(0.0f));
			}

			// Actors
			rapidxml::xml_node<> *actorsNode = rootNode->first_node("Actors");
			if(actorsNode) {
				std::vector<rapidxml::xml_node<> *> actors = XMLUtils::getChilds(actorsNode, "Actor");
				std::vector<rapidxml::xml_node<> *>::iterator p;
				for(p = actors.begin(); p != actors.end(); ++p) {
					rapidxml::xml_node<> *actorNode = *p;

					std::string type = XMLUtils::getAttribute(actorNode, "type", "");
					ActorType atype = Utils::toActorType(type.c_str());

					std::string primitive = XMLUtils::getAttribute(actorNode, "primitive", "");

					physx::PxVec3 pos = Utils::toVec3(XMLUtils::getAttribute(actorNode, "pos", "0, 0, 0"), physx::PxVec3(0.0f));
					physx::PxVec3 size = Utils::toVec3(XMLUtils::getAttribute(actorNode, "size", "0, 0, 0"), physx::PxVec3(0.0f));

					physx::PxVec4 color = Utils::toVec4(XMLUtils::getAttribute(actorNode, "color", "1, 1, 1, 1"));
					physx::PxVec3 velocity = Utils::toVec3(XMLUtils::getAttribute(actorNode, "vel", "0, 0, 0"), physx::PxVec3(0.0f));
					physx::PxVec3 rotate = Utils::toVec3(XMLUtils::getAttribute(actorNode, "rotate", "0, 0, 0"), physx::PxVec3(0.0f));

					std::string defaultDensity = Utils::toString(scene->defaultActorDensity);

					int actorTime = Utils::toInt(XMLUtils::getAttribute(actorNode, "time", "0"));
					float density = Utils::toFloat(XMLUtils::getAttribute(actorNode, "density", defaultDensity.c_str()));
					float radius = Utils::toFloat(XMLUtils::getAttribute(actorNode, "radius", "0.5"));
					bool visible = Utils::toBool(XMLUtils::getAttribute(actorNode, "visible", "true"));
					bool blending = Utils::toBool(XMLUtils::getAttribute(actorNode, "blending", atype == ActorType::ActorTypeStatic ? "true" : "false"));
					bool particleDrain = Utils::toBool(XMLUtils::getAttribute(actorNode, "particleDrain", "false"));

					CActor *newactor = NULL;
					if(strcmp(primitive.c_str(), "cube") == 0) {
						CCubeActor *typedActor = new CCubeActor(atype);
						typedActor->size = size;
						newactor = typedActor;
					} else if(strcmp(primitive.c_str(), "sphere") == 0) {
						CSphereActor *typedActor = new CSphereActor(atype);
						typedActor->radius = radius;
						newactor = typedActor;
					} else {
						std::cerr << "    Actor primitive type '" << primitive << "' is not valid!" << std::endl;
					}

					newactor->pos = pos;
					newactor->time = actorTime;
					newactor->color = color;
					newactor->density = density;
					newactor->velocity = velocity;
					newactor->visible = visible;
					newactor->blending = blending;
					newactor->rotate = rotate;
					newactor->particleDrain = particleDrain;

					if(newactor) {
						newScenario->addActor(newactor);
					}

				}
			}

			// Fluids
			rapidxml::xml_node<> *fluidsNode = rootNode->first_node("Fluids");
			if(fluidsNode) {
				std::vector<rapidxml::xml_node<> *> fluids = XMLUtils::getChilds(fluidsNode, "Fluid");
				std::vector<rapidxml::xml_node<> *>::iterator p;
				for(p = fluids.begin(); p != fluids.end(); ++p) {
					rapidxml::xml_node<> *fluidNode = *p;
					std::string fluidTypeStr = XMLUtils::getAttribute(fluidNode, "type", "blob");
					FluidType fluidType = Utils::toFluidType(fluidTypeStr.c_str());
					physx::PxVec3 pos = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "pos", "0, 0, 0"), physx::PxVec3(0.0f));
					physx::PxVec3 size = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "size", "0, 0, 0"), physx::PxVec3(0.0f));
					physx::PxVec3 velocity = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "vel", "0, 0, 0"), physx::PxVec3(0.0f));
					int fluidTime = Utils::toInt(XMLUtils::getAttribute(fluidNode, "time", "0"));
					float radius = Utils::toFloat(XMLUtils::getAttribute(fluidNode, "radius", "0.0"));
					bool isEmitter = Utils::toBool(XMLUtils::getAttribute(fluidNode, "isEmitter", "false"));
					float emitterRate = Utils::toFloat(XMLUtils::getAttribute(fluidNode, "emitterRate", "0.0"));
					float emitterTime = emitterRate > 0.0f ? 1000.0f / emitterRate : 0.0f;
					uint32_t emitterDuration = Utils::toUInt(XMLUtils::getAttribute(fluidNode, "emitterDuration", "0"));
					uint32_t emitterCoolDown = Utils::toUInt(XMLUtils::getAttribute(fluidNode, "emitterCoolDown", "0"));
					FluidContainer *fluidCon = new FluidContainer(pos, size, fluidType);
					fluidCon->vel = velocity;
					fluidCon->time = fluidTime;
					fluidCon->radius = radius;
					fluidCon->isEmitter = isEmitter;
					fluidCon->emitterRate = emitterRate;
					fluidCon->emitterTime = emitterTime;
					fluidCon->emitterDuration = emitterDuration;
					fluidCon->emitterCoolDown = emitterCoolDown;
					newScenario->addFluidContainer(fluidCon);
				}
			}
		}

		return newScenario;
	} else {
		return NULL;
	}
}