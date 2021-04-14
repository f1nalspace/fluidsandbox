#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "Singleton.hpp"

class COSLowLevel : public CSingleton<COSLowLevel>
{
friend CSingleton<COSLowLevel>;
public:
	COSLowLevel(void);
	uint32_t getNumCPUCores();
	const std::string getTextFileContent(const std::string &filename);
	const uint8_t *getBinaryFileContent(const std::string &filename);
	bool fileExists(const char* filename);
	std::vector<std::string> getFilesInDirectory(const std::string &str);
	double getTimeMilliSeconds();
	const std::string getAppPath(const int argc, char** argv);
	const std::string pathCombine(const std::string p1, const std::string p2);
};

