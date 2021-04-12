#include "Scene.h"

#include <iostream>

#include <rapidxml/rapidxml.hpp>

CScene::CScene(const float fluidViscosity, const float fluidStiffness, const float fluidParticleDistanceFactor, const float fluidParticleRenderFactor, const float fluidParticleRadius, const float fluidParticleMinDensity, const float defaultActorDensity)
{
	fluidRestitution = 0.3f;
	fluidDamping = 0.0f;
	fluidDynamicFriction = 0.001f;
	fluidMaxMotionDistance = 0.3f;
	fluidRestOffset = 0.12f;
	fluidContactOffset = 0.036f;
	fluidParticleMass = 0.005f;
	this->defaultFluidViscosity = fluidViscosity;
	this->defaultFluidStiffness = fluidStiffness;
	this->defaultFluidParticleDistanceFactor = fluidParticleDistanceFactor;
	this->defaultFluidParticleRenderFactor = fluidParticleRenderFactor;
	this->defaultFluidParticleRadius = fluidParticleRadius;
	this->defaultFluidParticleMinDensity = fluidParticleMinDensity;
	this->defaultActorDensity = defaultActorDensity;
	this->fluidViscosity = fluidViscosity;
	this->fluidStiffness = fluidStiffness;
	this->fluidParticleDistanceFactor = fluidParticleDistanceFactor;
	this->fluidParticleRenderFactor = fluidParticleRenderFactor;
	this->fluidParticleRadius = fluidParticleRadius;
	this->fluidParticleMinDensity = fluidParticleMinDensity;
	backgroundColor = physx::PxVec3(0.0f, 0.0f, 0.0f);
	numCPUThreads = 4;
	resetFluidColors();
}


CScene::~CScene(void)
{
}

void CScene::resetFluidColors()
{
	for (unsigned int i = 0; i < fluidColors.size(); i++) {
		FluidColor* col = fluidColors[i];
		delete col;
	}
	fluidColors.clear();

	addFluidColor(new FluidColor(physx::PxVec4(0.0f, 0.0f, 0.0f, 0.0f), physx::PxVec4(2.0f, 1.0f, 0.5f, 0.75f), true, "Clear"));
	addFluidColor(new FluidColor(physx::PxVec4(0.5f, 0.69f, 1.0f, 1.0f), physx::PxVec4(2.0f, 1.0f, 0.5f, 0.75f), false, "Blue"));
	addFluidColor(new FluidColor(physx::PxVec4(1.0f, 0.1f, 0.1f, 0.89f), physx::PxVec4(0.5f, 1.0f, 1.0f, 0.75f), false, "Red"));
	addFluidColor(new FluidColor(physx::PxVec4(0.69f, 1.0f, 0.5f, 1.0f), physx::PxVec4(1.0f, 0.25f, 1.0f, 0.75f), false, "Green"));
	addFluidColor(new FluidColor(physx::PxVec4(1.0f, 1.0f, 0.5f, 1.0f), physx::PxVec4(0.25f, 0.25f, 1.0f, 0.75f), false, "Yellow"));
	addFluidColor(new FluidColor(physx::PxVec4(0.0f, 1.0f, 0.5f, 1.0f), physx::PxVec4(0.25f, 0.25f, 1.0f, 0.75f), false, "Yellow 2"));
	fluidColorDefaultIndex = 0;
}

void CScene::load(const char* filename)
{
	if (COSLowLevel::getInstance()->fileExists(filename))
	{
		std::cout << "  Load scene from file '" << filename << "'" << std::endl;

		fluidColors.clear();
		fluidColorDefaultIndex = 0;

		std::string xml = COSLowLevel::getInstance()->getTextFileContent(filename);
		std::vector<char> xml_copy = Utils::toCharVector(xml);

		rapidxml::xml_document<> doc;
		doc.parse<0>(&xml_copy[0]);
		rapidxml::xml_node<> *rootNode = doc.first_node("Scene");
		if (rootNode) {

			// System
			rapidxml::xml_node<> *systemNode = rootNode->first_node("System");
			if (systemNode) {
				numCPUThreads =	Utils::toInt(XMLUtils::findNodeValue(systemNode, "CPUThreads", "4"));
			}

			// Fluid colors
			rapidxml::xml_node<> *fluidColorsNode = rootNode->first_node("FluidColors");
			if (fluidColorsNode) {
				std::vector<rapidxml::xml_node<>*> colors = XMLUtils::getChilds(fluidColorsNode, "FluidColor");
				std::vector<rapidxml::xml_node<>*>::iterator p;
				int index = 0;
				for (p = colors.begin(); p!=colors.end(); ++p) {
					rapidxml::xml_node<> *colorNode = *p;
					bool isclear = Utils::toBool(XMLUtils::getAttribute(colorNode, "clear", "false"));
					physx::PxVec4 baseColor = Utils::toVec4(XMLUtils::getAttribute(colorNode, "base", "0.0, 0.0, 0.0, 0.0"));
					physx::PxVec4 falloff = Utils::toVec4(XMLUtils::getAttribute(colorNode, "falloff", "0.0, 0.0, 0.0, 0.0"));
					std::string name = XMLUtils::getAttribute(colorNode, "name", "");
					bool isdefault = Utils::toBool(XMLUtils::getAttribute(colorNode, "default", "false"));
					FluidColor* ncolor = new FluidColor(baseColor, falloff, isclear, name.c_str());
					ncolor->falloffScale = Utils::toFloat(XMLUtils::getAttribute(colorNode, "falloffScale", isclear ? "0.0" : "0.1"));
					fluidColors.push_back(ncolor);
					if (isdefault)
						fluidColorDefaultIndex = index;
					index++;
				}

			}
			if (fluidColors.size() == 0){
				std::cout << "    Warning: No fluid colors found, reset to default values" << std::endl;
				resetFluidColors();
			}

			// Fluid system
			rapidxml::xml_node<> *fluidSystemNode = rootNode->first_node("FluidSystem");
			if (fluidSystemNode)
			{
				fluidRestitution = XMLUtils::findNodeFloat(fluidSystemNode, "Restitution", 0.3f);
				fluidDamping = XMLUtils::findNodeFloat(fluidSystemNode, "Damping", 0.0f);
				fluidDynamicFriction = XMLUtils::findNodeFloat(fluidSystemNode, "DynamicFriction", 0.001f);
				fluidMaxMotionDistance = XMLUtils::findNodeFloat(fluidSystemNode, "MaxMotionDistance", 0.3f);
				fluidRestOffset = XMLUtils::findNodeFloat(fluidSystemNode, "RestOffset", 0.3f);
				fluidContactOffset = XMLUtils::findNodeFloat(fluidSystemNode, "ContactOffset", 2.0f);
				fluidParticleMass = XMLUtils::findNodeFloat(fluidSystemNode, "ParticleMass", 0.005f);
				fluidViscosity = XMLUtils::findNodeFloat(fluidSystemNode, "Viscosity", defaultFluidViscosity);
				fluidStiffness = XMLUtils::findNodeFloat(fluidSystemNode, "Stiffness", defaultFluidStiffness);
				fluidParticleRadius = XMLUtils::findNodeFloat(fluidSystemNode, "ParticleRadius", 0.05f);
				fluidParticleDistanceFactor = XMLUtils::findNodeFloat(fluidSystemNode, "ParticleDistanceFactor", defaultFluidParticleDistanceFactor);
				fluidParticleRenderFactor = XMLUtils::findNodeFloat(fluidSystemNode, "ParticleRenderFactor", defaultFluidParticleRenderFactor);
				fluidParticleMinDensity = XMLUtils::findNodeFloat(fluidSystemNode, "ParticleMinDensity", 0.01f);
			}

			// Properties
			rapidxml::xml_node<> *propertiesNode = rootNode->first_node("Properties");
			if (propertiesNode) {
				backgroundColor = Utils::toVec3(XMLUtils::findNodeValue(propertiesNode, "BackgroundColor", "0.0, 0.0, 0.0"), physx::PxVec3(0.0f));
			}
		}
	}
}