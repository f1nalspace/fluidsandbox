/*
======================================================================================================================
	Fluid Sandbox - OSLowLevel.h

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace COSLowLevel
{
	uint32_t getNumCPUCores();
	const std::string getTextFileContent(const std::string &filename);
	const uint8_t *getBinaryFileContent(const std::string &filename);
	bool fileExists(const char* filename);
	std::vector<std::string> getFilesInDirectory(const std::string &str);
	double getTimeMilliSeconds();
	const std::string getAppPath(const int argc, char** argv);
	const std::string pathCombine(const std::string p1, const std::string p2);
};

