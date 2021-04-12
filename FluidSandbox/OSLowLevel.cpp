#include "OSLowLevel.h"

#include <fstream>
#include <iostream>

COSLowLevel::COSLowLevel(void)
{
	qpcFrequency = 0;
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
	if (qpcFrequency == 0) {
		QueryPerformanceFrequency((LARGE_INTEGER*)&qpcFrequency);
	}
	LONGLONG cur = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&cur);
	return (double)(cur * 1000.0 / qpcFrequency);
}

const std::string COSLowLevel::getAppPath(const int argc, char** argv) {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
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