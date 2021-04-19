/*
======================================================================================================================
	Fluid Sandbox - OSLowLevel.cpp

	Copyright (C) Torsten Spaete 2011-2021. All rights reserved.
	MPL v2 licensed. See LICENSE.txt for more details.
======================================================================================================================
*/

#include "OSLowLevel.h"

#include <fstream>
#include <iostream>

#include "final_platform_layer.h"

#include "Utils.h"

namespace COSLowLevel {

	uint32_t COSLowLevel::getNumCPUCores() {
		uint32_t result = (uint32_t)fplCPUGetCoreCount();
		return result;
	}


	std::string COSLowLevel::getTextFileContent(const std::string &filePath) {
		std::string result = "";
		std::ifstream myfile(filePath);
		if(myfile && myfile.is_open()) {
			std::string line;
			while(myfile.good()) {
				std::getline(myfile, line);
				result += line + "\n";
			}
			myfile.close();
		}
		return result;
	}

	uint8_t *COSLowLevel::getBinaryFileContent(const std::string &filePath) {
		fplFileHandle file;
		if(fplOpenBinaryFile(filePath.c_str(), &file)) {
			size_t len = fplGetFileSizeFromHandle(&file);
			uint8_t *result = new uint8_t[len];
			fplReadFileBlock(&file, len, &result[0], len);
			return(result);
		} else {
			return(nullptr);
		}
	}

	bool COSLowLevel::fileExists(const char *filePath) {
		bool result = fplFileExists(filePath);
		return(result);
	}

	std::vector<std::string> COSLowLevel::getFilesInDirectory(const std::string &folderPath, const std::string &filter) {
		std::vector<std::string> result;
		fplFileEntry entry;
		for(bool isValid = fplListDirBegin(folderPath.c_str(), filter.c_str(), &entry); isValid; isValid = fplListDirNext(&entry)) {
			if(entry.type == fplFileEntryType_File) {
				result.push_back(std::string(entry.name));
			}
		}
		fplListDirEnd(&entry);
		return(result);
	}

	double COSLowLevel::getTimeMilliSeconds() {
		double result = fplGetTimeInMillisecondsHP();
		return(result);
	}

	std::string COSLowLevel::getAppPath(const int argc, char **argv) {
		size_t pathLen = fplGetExecutableFilePath(nullptr, 0) + 1;
		std::string fullPath;
		fullPath.reserve(pathLen);
		fplGetExecutableFilePath(&fullPath[0], pathLen);
		size_t appPathLen = fplExtractFilePath(fullPath.c_str(), nullptr, 0) + 1;
		std::string result;
		result.reserve(appPathLen);
		fplExtractFilePath(fullPath.c_str(), &result[0], appPathLen);
		return(result);
	}

	std::string COSLowLevel::pathCombine(const std::string &p1, const std::string &p2) {
		size_t len = fplPathCombine(nullptr, 0, 2, p1.c_str(), p2.c_str()) + 1;
		std::string result;
		result.reserve(len);
		fplPathCombine(&result[0], len, 2, p1.c_str(), p2.c_str());
		return(result);
	}

};