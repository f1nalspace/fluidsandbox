#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#ifdef WIN32
	#include <Windows.h>
#endif

#include "Singleton.hpp"

class COSLowLevel : public CSingleton<COSLowLevel>
{
friend CSingleton<COSLowLevel>;   
private:
	LONGLONG qpcFrequency;
public:
	COSLowLevel(void);
	int getNumCPUCores();
	string getTextFileContent(const char* filename);
	bool fileExists(const char* filename);
	vector<string> getFilesInDirectory(const string &str);
	double getTimeMilliSeconds();
	string getAppPath(const int argc, char** argv);
	string pathCombine(const string p1, const string p2);
};

