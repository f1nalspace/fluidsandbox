#include "OSLowLevel.h"

#include <fstream>
#include <iostream>

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

COSLowLevel::COSLowLevel(void)
{
}

uint32_t COSLowLevel::getNumCPUCores() 
{ 
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	uint32_t result = sysinfo.dwNumberOfProcessors;
	return result;
}


const std::string COSLowLevel::getTextFileContent(const std::string &filename)
{
	std::string result = "";
	std::ifstream myfile(filename);
	if (myfile && myfile.is_open())
	{
		std::string line;
		while ( myfile.good() )
		{
			std::getline (myfile,line);
			result += line + "\n";
		}
		myfile.close();
	}
	return result;
}

const uint8_t *COSLowLevel::getBinaryFileContent(const std::string &filename) {
	std::ifstream myfile(filename, std::ios::binary | std::ios::ate);
	if(myfile && myfile.is_open()) {
		size_t size = (size_t)myfile.tellg();
		uint8_t *result = new uint8_t[size];
		myfile.seekg(0, std::ios::beg);
		myfile.read((char *)result, size);
		myfile.close();
	}
	return nullptr;
}

bool COSLowLevel::fileExists(const char* filename)
{
	std::ifstream myfile(filename);
	return !myfile.fail();
}

std::vector<std::string> COSLowLevel::getFilesInDirectory(const std::string &str)
{
	std::vector<std::string> r;

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFileA(str.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) {
		r.push_back(FindFileData.cFileName);
		while (FindNextFileA(hFind, &FindFileData)) {
			r.push_back(FindFileData.cFileName);
		}
		FindClose(hFind);
	}

	return r;
}

double COSLowLevel::getTimeMilliSeconds() {
	LARGE_INTEGER freq, cur;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&cur);
	return (double)(cur.QuadPart * 1000.0 / freq.QuadPart);
}

const std::string COSLowLevel::getAppPath(const int argc, char** argv) {
	char buffer[MAX_PATH];
	GetModuleFileNameA(nullptr, buffer, MAX_PATH);
	std::string fullpath = buffer;
	return std::string(fullpath, 0, fullpath.rfind("\\"));
}

const std::string COSLowLevel::pathCombine(const std::string p1, const std::string p2) {
	std::string s = p1;
	if (s.length() > 0 && s.compare(s.length()-1, 1, "\\") != 0) {
		s += "\\";
	}
	std::string a = p2;
	if (a.length() > 0 && a.compare(0, 1, "\\") == 0) {
		a = "\\" + a;
	}
	return s + a;
}