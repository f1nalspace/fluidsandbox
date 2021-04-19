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
	std::string getTextFileContent(const std::string &filePath);
	uint8_t *getBinaryFileContent(const std::string &filePath);
	bool fileExists(const char* filePath);
	std::vector<std::string> getFilesInDirectory(const std::string &folderPath, const std::string &filter);
	double getTimeMilliSeconds();
	std::string getAppPath(const int argc, char** argv);
	std::string pathCombine(const std::string &p1, const std::string &p2);
};

