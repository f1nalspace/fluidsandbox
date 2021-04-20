/*
======================================================================================================================
	Fluid Sandbox - Scene.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "Scene.h"

#include <iostream>

#include <final_xml.h>

#include "OSLowLevel.h"

CScene::CScene(const float defaultActorDensity) {
	sim = FluidSimulationProperties::Compute(FluidSimulationProperties::DefaultParticleRadius, FluidSimulationProperties::DefaultParticleRestDistanceFactor);

	render = FluidRenderProperties();
	render.particleRenderFactor = FluidRenderProperties::DefaultParticleRenderFactor;
	render.minDensity = FluidRenderProperties::DefaultMinDensity;

	this->defaultActorDensity = defaultActorDensity;

	backgroundColor = glm::vec3(0.0f, 0.0f, 0.0f);
	numCPUThreads = 4;
	resetFluidColors();
}


CScene::~CScene(void) {
}

void CScene::resetFluidColors() {
	fluidColors.clear();
	addFluidColor(FluidColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(2.0f, 1.0f, 0.5f, 0.75f), true, "Clear"));
	addFluidColor(FluidColor(glm::vec4(0.5f, 0.69f, 1.0f, 1.0f), glm::vec4(2.0f, 1.0f, 0.5f, 0.75f), false, "Blue"));
	addFluidColor(FluidColor(glm::vec4(1.0f, 0.1f, 0.1f, 0.89f), glm::vec4(0.5f, 1.0f, 1.0f, 0.75f), false, "Red"));
	addFluidColor(FluidColor(glm::vec4(0.69f, 1.0f, 0.5f, 1.0f), glm::vec4(1.0f, 0.25f, 1.0f, 0.75f), false, "Green"));
	addFluidColor(FluidColor(glm::vec4(1.0f, 1.0f, 0.5f, 1.0f), glm::vec4(0.25f, 0.25f, 1.0f, 0.75f), false, "Yellow"));
	addFluidColor(FluidColor(glm::vec4(0.0f, 1.0f, 0.5f, 1.0f), glm::vec4(0.25f, 0.25f, 1.0f, 0.75f), false, "Yellow 2"));
	fluidColorDefaultIndex = 0;
}

bool CScene::load(const char *filePath) {
	if(!COSLowLevel::fileExists(filePath)) {
		std::cout << "The scene file '" << filePath << "' was not found!" << std::endl;
		return(false);
	}

	fluidColors.clear();
	fluidColorDefaultIndex = 0;

	std::cout << "Load scene from file '" << filePath << "'" << std::endl;
	std::string xml = COSLowLevel::getTextFileContent(filePath);
	std::cout << "Successfully loaded scene from file '" << filePath << "' with length of " << xml.length() << std::endl;

	// Parse XML
	fxmlContext ctx = FXML_ZERO_INIT;
	fxmlTag root = FXML_ZERO_INIT;
	size_t xmlLen = xml.length();
	if(!fxmlInitFromMemory(xml.c_str(), xmlLen, &ctx)) {
		std::cerr << "Failed initialize XML context with xml length of " << xml << std::endl;
		return(false);
	}
	if(!fxmlParse(&ctx, &root)) {
		std::cerr << "Failed to parse XML file '" << filePath << "'!" << std::endl;
		return(false);
	}

	const fxmlTag *rootNode = fxmlFindTagByName(&root, "Scene");
	if(!rootNode) {
		std::cerr << "The scene node in the xml file '" << filePath << "' was not found!" << std::endl;
		fxmlFree(&ctx);
		return(false);
	}

	XMLUtils xmlUtils = XMLUtils();

	// System
	const fxmlTag *systemNode = fxmlFindTagByName(rootNode, "System");
	if(systemNode) {
		numCPUThreads = xmlUtils.getNodeS32(systemNode, "CPUThreads", COSLowLevel::getNumCPUCores());
	}

	// Fluid colors
	const fxmlTag *fluidColorsNode = fxmlFindTagByName(rootNode, "FluidColors");
	if(fluidColorsNode) {
		std::vector<const fxmlTag *> colors = xmlUtils.getChilds(fluidColorsNode, "FluidColor");
		int index = 0;
		for(auto p = colors.begin(); p != colors.end(); ++p) {
			const fxmlTag *colorNode = *p;
			bool isclear = xmlUtils.getAttributeBool(colorNode, "clear", false);
			glm::vec4 baseColor = xmlUtils.getAttributeVec4(colorNode, "base", glm::vec4(0.0f));
			glm::vec4 falloff = xmlUtils.getAttributeVec4(colorNode, "falloff", glm::vec4(0.0f));
			std::string name = xmlUtils.getAttribute(colorNode, "name", "");
			bool isDefault = xmlUtils.getAttributeBool(colorNode, "default", false);
			FluidColor fluidColor = FluidColor(baseColor, falloff, isclear, name.c_str());
			fluidColor.falloffScale = xmlUtils.getAttributeFloat(colorNode, "falloffScale", isclear ? 0.0f : 0.1f);
			fluidColors.push_back(fluidColor);
			if(isDefault)
				fluidColorDefaultIndex = index;
			index++;
		}

	}
	if(fluidColors.size() == 0) {
		std::cout << "    Warning: No fluid colors found, reset to default values" << std::endl;
		resetFluidColors();
	}

	// Fluid system
	const fxmlTag *fluidSystemNode = fxmlFindTagByName(rootNode, "FluidSystem");
	if(fluidSystemNode) {
		const float particleDistanceFactor = xmlUtils.getNodeFloat(fluidSystemNode, "ParticleDistanceFactor", FluidSimulationProperties::DefaultParticleRestDistanceFactor);
		const float particleRadius = xmlUtils.getNodeFloat(fluidSystemNode, "ParticleRadius", FluidSimulationProperties::DefaultParticleRadius);

		sim = FluidSimulationProperties::Compute(particleRadius, particleDistanceFactor);
		sim.restitution = xmlUtils.getNodeFloat(fluidSystemNode, "Restitution", FluidSimulationProperties::DefaultRestitution);
		sim.damping = xmlUtils.getNodeFloat(fluidSystemNode, "Damping", FluidSimulationProperties::DefaultDamping);
		sim.dynamicFriction = xmlUtils.getNodeFloat(fluidSystemNode, "DynamicFriction", FluidSimulationProperties::DefaultDynamicFriction);
		sim.maxMotionDistance = xmlUtils.getNodeFloat(fluidSystemNode, "MaxMotionDistance", FluidSimulationProperties::DefaultMaxMotionDistance);
		sim.restOffset = xmlUtils.getNodeFloat(fluidSystemNode, "RestOffset", FluidSimulationProperties::DefaultRestOffset);
		sim.contactOffset = xmlUtils.getNodeFloat(fluidSystemNode, "ContactOffset", FluidSimulationProperties::DefaultContactOffset);
		sim.particleMass = xmlUtils.getNodeFloat(fluidSystemNode, "ParticleMass", FluidSimulationProperties::DefaultParticleMass);
		sim.viscosity = xmlUtils.getNodeFloat(fluidSystemNode, "Viscosity", FluidSimulationProperties::DefaultViscosity);
		sim.stiffness = xmlUtils.getNodeFloat(fluidSystemNode, "Stiffness", FluidSimulationProperties::DefaultStiffness);

		render.particleRenderFactor = xmlUtils.getNodeFloat(fluidSystemNode, "ParticleRenderFactor", FluidRenderProperties::DefaultParticleRenderFactor);
		render.minDensity = xmlUtils.getNodeFloat(fluidSystemNode, "ParticleMinDensity", FluidRenderProperties::DefaultMinDensity);
	}

	// Properties
	const fxmlTag *propertiesNode = fxmlFindTagByName(rootNode, "Properties");
	if(propertiesNode) {
		backgroundColor = xmlUtils.getNodeVec3(propertiesNode, "BackgroundColor", glm::vec3(0.0f));
	}

	return(true);
}