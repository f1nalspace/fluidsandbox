#include "FluidScenario.h"

#include <iostream>

#include <glm/glm.hpp>

#include <rapidxml/rapidxml.hpp>

#include "OSLowLevel.h"
#include "XMLUtils.h"
#include "Utils.h"

#include "AllActors.hpp"

FluidScenario::FluidScenario() {
	this->name[0] = 0;
	this->actorCreatePosition = glm::vec3(0.0f, 0.0f, 0.0f);
	this->sim = FluidSimulationProperties();
#if 0
	this->viscosity = 20.0f;
	this->stiffness = 35.0f;
	this->damping = 0.0f;
	this->particleDistanceFactor = 2.0f;
	this->particleRenderFactor = 1.0f;
	this->particleRadius = 0.05f;
	this->particleMinDensity = 0.01f;
#endif
	this->gravity = glm::vec3(0.0f, -9.8f, 0.0f);
}


FluidScenario::~FluidScenario(void) {
	for(size_t i = 0; i < fluidContainers.size(); i++) {
		FluidContainer *container = fluidContainers[i];
		delete container;
	}
	fluidContainers.clear();

	for(size_t i = 0; i < actors.size(); i++) {
		Actor *actor = actors[i];
		delete actor;
	}
	actors.clear();
}

FluidScenario *FluidScenario::load(const char *filename, CScene *scene) {
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

		FluidScenario *newScenario = new FluidScenario();

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
				newScenario->gravity = Utils::toVec3(gravitiyNode->value(), glm::vec3(0.0f, -9.8f, 0.0f));
			}

			// Fluid properties
			rapidxml::xml_node<> *fpNode = rootNode->first_node("FluidProperties");
			if(fpNode) {
				float particleRadius = XMLUtils::findNodeFloat(fpNode, "ParticleRadius", scene->sim.particleRadius);
				float particleDistanceFactor = XMLUtils::findNodeFloat(fpNode, "ParticleDistanceFactor", scene->sim.particleDistanceFactor);

				newScenario->sim = FluidSimulationProperties::Compute(particleRadius, particleDistanceFactor);
				newScenario->render = scene->render;

				newScenario->sim.viscosity = XMLUtils::findNodeFloat(fpNode, "Viscosity", scene->sim.viscosity);
				newScenario->sim.stiffness = XMLUtils::findNodeFloat(fpNode, "Stiffness", scene->sim.stiffness);
				newScenario->sim.damping = XMLUtils::findNodeFloat(fpNode, "Damping", scene->sim.damping);

				newScenario->render.particleRenderFactor = XMLUtils::findNodeFloat(fpNode, "ParticleRenderFactor", scene->render.particleRenderFactor);
				newScenario->render.minDensity = XMLUtils::findNodeFloat(fpNode, "ParticleMinDensity", scene->render.minDensity);
			} else {
				newScenario->sim = scene->sim;
				newScenario->render = scene->render;
			}

			// Actor properties
			rapidxml::xml_node<> *apNode = rootNode->first_node("ActorProperties");
			if(apNode) {
				newScenario->actorCreatePosition = Utils::toVec3(XMLUtils::findNodeValue(apNode, "CreatePosition", "0, 0, 0"), glm::vec3(0.0f));
			}

			// Actors
			rapidxml::xml_node<> *actorsNode = rootNode->first_node("Actors");
			if(actorsNode) {
				std::vector<rapidxml::xml_node<> *> actors = XMLUtils::getChilds(actorsNode, "Actor");
				std::vector<rapidxml::xml_node<> *>::iterator p;
				for(p = actors.begin(); p != actors.end(); ++p) {
					rapidxml::xml_node<> *actorNode = *p;

					std::string type = XMLUtils::getAttribute(actorNode, "type", "");
					ActorMovementType atype = Utils::toActorMovementType(type.c_str());

					std::string primitive = XMLUtils::getAttribute(actorNode, "primitive", "");

					glm::vec3 pos = Utils::toVec3(XMLUtils::getAttribute(actorNode, "pos", "0, 0, 0"), glm::vec3(0.0f));
					glm::vec3 size = Utils::toVec3(XMLUtils::getAttribute(actorNode, "size", "0, 0, 0"), glm::vec3(0.0f));

					glm::vec4 color = Utils::toVec4(XMLUtils::getAttribute(actorNode, "color", "1, 1, 1, 1"));
					glm::vec3 velocity = Utils::toVec3(XMLUtils::getAttribute(actorNode, "vel", "0, 0, 0"), glm::vec3(0.0f));
					glm::vec3 rotate = Utils::toVec3(XMLUtils::getAttribute(actorNode, "rotate", "0, 0, 0"), glm::vec3(0.0f));

					std::string defaultDensity = Utils::toString(scene->defaultActorDensity);

					std::string defaultBlending = (atype == ActorMovementType::Static) ? "true" : "false";

					int actorTime = Utils::toInt(XMLUtils::getAttribute(actorNode, "time", "0"));
					float density = Utils::toFloat(XMLUtils::getAttribute(actorNode, "density", defaultDensity.c_str()));
					float radius = Utils::toFloat(XMLUtils::getAttribute(actorNode, "radius", "0.5"));
					bool visible = Utils::toBool(XMLUtils::getAttribute(actorNode, "visible", "true"));
					bool blending = Utils::toBool(XMLUtils::getAttribute(actorNode, "blending", defaultBlending.c_str()));
					bool particleDrain = Utils::toBool(XMLUtils::getAttribute(actorNode, "particleDrain", "false"));

					Actor *newactor = NULL;
					if(strcmp(primitive.c_str(), "cube") == 0) {
						CubeActor *typedActor = new CubeActor(atype);
						typedActor->size = size;
						newactor = typedActor;
					} else if(strcmp(primitive.c_str(), "sphere") == 0) {
						SphereActor *typedActor = new SphereActor(atype);
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
					newactor->rotation = rotate;
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
					glm::vec3 pos = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "pos", "0, 0, 0"), glm::vec3(0.0f));
					glm::vec3 size = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "size", "0, 0, 0"), glm::vec3(0.0f));
					glm::vec3 velocity = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "vel", "0, 0, 0"), glm::vec3(0.0f));
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