#include "FluidScenario.h"

CFluidScenario::CFluidScenario()
{
	this->name = "";
	this->actorCreatePosition = PxVec3(0.0f, 0.0f, 0.0f);
	this->viscosity = 20.0f;
	this->stiffness = 35.0f;
	this->damping = 0.0f;
	this->particleDistanceFactor = 2.0f;
	this->particleRenderFactor = 1.0f;
	this->particleRadius = 0.05f;
	this->particleMinDensity = 0.01f;
	this->gravity = PxVec3(0.0f, -9.8f, 0.0f);
}


CFluidScenario::~CFluidScenario(void)
{
	for (unsigned int i = 0; i < fluidContainers.size(); i++) {
		FluidContainer *container = fluidContainers[i];
		delete container;
	}
	fluidContainers.clear();

	for (unsigned int i = 0; i < actors.size(); i++) {
		CActor *actor = actors[i];
		delete actor;
	}
	actors.clear();
}

CFluidScenario* CFluidScenario::load(const char* filename, CScene* scene)
{
	if (COSLowLevel::getInstance()->fileExists(filename))
	{
		cout << "  Load scenario from file '" << filename << "'" << endl;

		string xml = COSLowLevel::getInstance()->getTextFileContent(filename);
		vector<char> xml_copy = Utils::toCharVector(xml);

		map<string, string> variables;

		// First get variables
		xml_document<> doc;
		try{
			doc.parse<0>(&xml_copy[0]);
		}
		catch (...) {
			cerr << "could not parse xml!" << endl;
			return NULL;
		}
		xml_node<> *rootNode = doc.first_node("Scenario");
		if (rootNode) {
			xml_node<> *varsNode = rootNode->first_node("Variables");
			if (varsNode) {
				vector<xml_node<>*> childs = XMLUtils::getChilds(varsNode);
				vector<xml_node<>*>::iterator p;
				for (p = childs.begin(); p!=childs.end(); ++p) {
					xml_node<> *varNode = *p;
					variables.insert(pair<string, string>(varNode->name(), varNode->value()));
				}
			}
		}

		// Now replace all strings in xml content
		map<string, string>::iterator p;
		for (p = variables.begin(); p != variables.end(); ++p) {
			string name = p->first;
			name = "{%" + name + "}";
			string value = p->second;
			Utils::replaceString(xml, name, value);
		}

		// Now reload xml again
		doc.clear();
		xml_copy = Utils::toCharVector(xml);
		doc.parse<0>(&xml_copy[0]);

		CFluidScenario* newScenario = new CFluidScenario();

		rootNode = doc.first_node("Scenario");
		if (rootNode) {
			// Name
			xml_node<> *nameNode = rootNode->first_node("Name");
			if (nameNode) {
				newScenario->setName(nameNode->value());
			}

			// Gravity
			xml_node<> *gravitiyNode = rootNode->first_node("Gravity");
			if (gravitiyNode) {
				newScenario->setGravity(Utils::toVec3(gravitiyNode->value(), PxVec3(0.0f, -9.8f, 0.0f)));
			}

			// Fluid properties
			xml_node<> *fpNode = rootNode->first_node("FluidProperties");
			if (fpNode) {
				newScenario->setViscosity(XMLUtils::findNodeFloat(fpNode, "Viscosity", scene->getFluidViscosity()));
				newScenario->setStiffness(XMLUtils::findNodeFloat(fpNode, "Stiffness", scene->getFluidStiffness()));
				newScenario->setDamping(XMLUtils::findNodeFloat(fpNode, "Damping", scene->getFluidDamping()));
				newScenario->setParticleDistanceFactor(XMLUtils::findNodeFloat(fpNode, "ParticleDistanceFactor", scene->getFluidParticleDistanceFactor()));
				newScenario->setParticleRenderFactor(XMLUtils::findNodeFloat(fpNode, "ParticleRenderFactor", scene->getFluidParticleRenderFactor()));
				newScenario->setParticleRadius(XMLUtils::findNodeFloat(fpNode, "ParticleRadius", scene->getFluidParticleRadius()));
				newScenario->setParticleMinDensity(XMLUtils::findNodeFloat(fpNode, "ParticleMinDensity", scene->getFluidParticleMinDensity()));
			} else {
				newScenario->setViscosity(scene->getFluidViscosity());
				newScenario->setStiffness(scene->getFluidStiffness());
				newScenario->setDamping(scene->getFluidDamping());
				newScenario->setParticleDistanceFactor(scene->getFluidParticleDistanceFactor());
				newScenario->setParticleRenderFactor(scene->getFluidParticleRenderFactor());
				newScenario->setParticleRadius(scene->getFluidParticleRadius());
				newScenario->setParticleMinDensity(scene->getFluidParticleMinDensity());
			}

			// Actor properties
			xml_node<> *apNode = rootNode->first_node("ActorProperties");
			if (apNode) {
				newScenario->setActorCreatePosition(Utils::toVec3(XMLUtils::findNodeValue(apNode, "CreatePosition", "0, 0, 0"), PxVec3(0.0f)));
			}

			// Actors
			xml_node<> *actorsNode = rootNode->first_node("Actors");
			if (actorsNode) {
				vector<xml_node<>*> actors = XMLUtils::getChilds(actorsNode, "Actor");
				vector<xml_node<>*>::iterator p;
				for (p = actors.begin(); p!=actors.end(); ++p) {
					xml_node<> *actorNode = *p;

					string type = XMLUtils::getAttribute(actorNode, "type", "");
					EActorType atype = Utils::toActorType(type.c_str());

					string primitive = XMLUtils::getAttribute(actorNode, "primitive", "");

					PxVec3 pos = Utils::toVec3(XMLUtils::getAttribute(actorNode, "pos", "0, 0, 0"), PxVec3(0.0f));
					PxVec3 size = Utils::toVec3(XMLUtils::getAttribute(actorNode, "size", "0, 0, 0"), PxVec3(0.0f));

					PxVec4 color = Utils::toVec4(XMLUtils::getAttribute(actorNode, "color", "1, 1, 1, 1"));
					PxVec3 velocity = Utils::toVec3(XMLUtils::getAttribute(actorNode, "vel", "0, 0, 0"), PxVec3(0.0f));
					PxVec3 rotate = Utils::toVec3(XMLUtils::getAttribute(actorNode, "rotate", "0, 0, 0"), PxVec3(0.0f));

					string defaultDensity = Utils::toString(scene->getDefaultActorDensity());

					int actorTime = Utils::toInt(XMLUtils::getAttribute(actorNode, "time", "0"));
					float density = Utils::toFloat(XMLUtils::getAttribute(actorNode, "density", defaultDensity.c_str()));
					float radius = Utils::toFloat(XMLUtils::getAttribute(actorNode, "radius", "0.5"));
					bool visible = Utils::toBool(XMLUtils::getAttribute(actorNode, "visible", "true"));
					bool blending = Utils::toBool(XMLUtils::getAttribute(actorNode, "blending", atype == ActorTypeStatic ? "true" : "false"));
					bool particleDrain = Utils::toBool(XMLUtils::getAttribute(actorNode, "particleDrain", "false"));

					CActor* newactor = NULL;
					if (strcmp(primitive.c_str(), "cube") == 0) {
						CCubeActor* typedActor = new CCubeActor(atype);
						typedActor->setSize(size);
						newactor = typedActor;
					} else if (strcmp(primitive.c_str(), "sphere") == 0) {
						CSphereActor* typedActor = new CSphereActor(atype);
						typedActor->setRadius(radius);
						newactor = typedActor;
					} else {
						cerr << "    Actor primitive type '" << primitive << "' is not valid!" << endl;
					}

					newactor->setPos(pos);
					newactor->setTime(actorTime);
					newactor->setColor(color);
					newactor->setDensity(density);
					newactor->setVelocity(velocity);
					newactor->setVisible(visible);
					newactor->setBlending(blending);
					newactor->setRotate(rotate);
					newactor->setParticleDrain(particleDrain);

					if (newactor) {
						newScenario->addActor(newactor);
					}

				}
			}

			// Fluids
			xml_node<> *fluidsNode = rootNode->first_node("Fluids");
			if (fluidsNode) {
				vector<xml_node<>*> fluids = XMLUtils::getChilds(fluidsNode, "Fluid");
				vector<xml_node<>*>::iterator p;
				for (p = fluids.begin(); p!=fluids.end(); ++p) {
					xml_node<> *fluidNode = *p;
					string fluidTypeStr = XMLUtils::getAttribute(fluidNode, "type", "blob");
					FluidType fluidType = Utils::toFluidType(fluidTypeStr.c_str());
					PxVec3 pos = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "pos", "0, 0, 0"), PxVec3(0.0f));
					PxVec3 size = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "size", "0, 0, 0"), PxVec3(0.0f));
					PxVec3 velocity = Utils::toVec3(XMLUtils::getAttribute(fluidNode, "vel", "0, 0, 0"), PxVec3(0.0f));
					int fluidTime = Utils::toInt(XMLUtils::getAttribute(fluidNode, "time", "0"));
					float radius = Utils::toFloat(XMLUtils::getAttribute(fluidNode, "radius", "0.0"));
					bool isEmitter = Utils::toBool(XMLUtils::getAttribute(fluidNode, "isEmitter", "false"));
					float emitterRate = Utils::toFloat(XMLUtils::getAttribute(fluidNode, "emitterRate", "0.0"));
					float emitterTime = emitterRate > 0.0f ? 1000.0f / emitterRate : 0.0f;
					unsigned int emitterDuration = Utils::toUInt(XMLUtils::getAttribute(fluidNode, "emitterDuration", "0"));
					unsigned int emitterCoolDown = Utils::toUInt(XMLUtils::getAttribute(fluidNode, "emitterCoolDown", "0"));
					FluidContainer *fluidCon = new FluidContainer(pos, size, fluidType);
					fluidCon->Vel = velocity;
					fluidCon->Time = fluidTime;
					fluidCon->Radius = radius;
					fluidCon->IsEmitter = isEmitter;
					fluidCon->EmitterRate = emitterRate;
					fluidCon->EmitterTime = emitterTime;
					fluidCon->EmitterDuration = emitterDuration;
					fluidCon->EmitterCoolDown = emitterCoolDown;
					newScenario->addFluidContainer(fluidCon);
				}
			}
		}

		return newScenario;
	} else {
		return NULL;
	}
}