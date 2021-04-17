/*
======================================================================================================================
	Fluid Sandbox - Scene.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "Scene.h"

#include <iostream>

#include <rapidxml/rapidxml.hpp>

CScene::CScene(const float fluidParticleRadius, const float fluidViscosity, const float fluidStiffness, const float fluidParticleDistanceFactor, const float fluidParticleRenderFactor, const float fluidParticleMinDensity, const float defaultActorDensity) {
	sim = FluidSimulationProperties::Compute(fluidParticleRadius, fluidParticleDistanceFactor);
	sim.viscosity = fluidViscosity;
	sim.stiffness = fluidStiffness;

	render = FluidRenderProperties();
	render.particleRenderFactor = fluidParticleRenderFactor;
	render.minDensity = fluidParticleMinDensity;

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

void CScene::load(const char *filename) {
	if(COSLowLevel::fileExists(filename)) {
		std::cout << "  Load scene from file '" << filename << "'" << std::endl;

		fluidColors.clear();
		fluidColorDefaultIndex = 0;

		std::string xml = COSLowLevel::getTextFileContent(filename);
		std::vector<char> xml_copy = Utils::toCharVector(xml);

		rapidxml::xml_document<> doc;
		doc.parse<0>(&xml_copy[0]);
		rapidxml::xml_node<> *rootNode = doc.first_node("Scene");
		if(rootNode) {

			XMLUtils xmlUtils = XMLUtils();

			// System
			rapidxml::xml_node<> *systemNode = rootNode->first_node("System");
			if(systemNode) {
				numCPUThreads = xmlUtils.getNodeS32(systemNode, "CPUThreads", 1);
			}

			// Fluid colors
			rapidxml::xml_node<> *fluidColorsNode = rootNode->first_node("FluidColors");
			if(fluidColorsNode) {
				std::vector<rapidxml::xml_node<> *> colors = xmlUtils.getChilds(fluidColorsNode, "FluidColor");
				int index = 0;
				for(auto p = colors.begin(); p != colors.end(); ++p) {
					rapidxml::xml_node<> *colorNode = *p;
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
			rapidxml::xml_node<> *fluidSystemNode = rootNode->first_node("FluidSystem");
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
			rapidxml::xml_node<> *propertiesNode = rootNode->first_node("Properties");
			if(propertiesNode) {
				backgroundColor = xmlUtils.getNodeVec3(propertiesNode, "BackgroundColor", glm::vec3(0.0f));
			}
		}
	}
}