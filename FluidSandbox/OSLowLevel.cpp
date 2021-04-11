#include "OSLowLevel.h"

COSLowLevel::COSLowLevel(void)
{
	qpcFrequency = 0;
}

int COSLowLevel::getNumCPUCores() 
{ 
	int nNumCPUCores = 1;
	#ifdef WIN32
		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );
		nNumCPUCores = sysinfo.dwNumberOfProcessors;
	#endif
	return nNumCPUCores;
}


string COSLowLevel::getTextFileContent(const char* filename)
{
	string result = "";
	ifstream myfile(filename);
	if (myfile && myfile.is_open())
	{
		string line;
		while ( myfile.good() )
		{
			getline (myfile,line);
			result += line + "\n";
		}
		myfile.close();
	}
	return result;
}

bool COSLowLevel::fileExists(const char* filename)
{
	ifstream myfile(filename);
	return !myfile.fail();
}

vector<string> COSLowLevel::getFilesInDirectory(const string &str)
{
	vector<string> r;

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

string COSLowLevel::getAppPath(const int argc, char** argv) {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	string fullpath = buffer;
	return string(fullpath, 0, fullpath.rfind("\\"));
}

string COSLowLevel::pathCombine(const string p1, const string p2) {
	string s = p1;
	if (s.length() > 0 && s.compare(s.length()-1, 1, "\\") != 0) {
		s += "\\";
	}
	string a = p2;
	if (a.length() > 0 && a.compare(0, 1, "\\") == 0) {
		a = "\\" + a;
	}
	return s + a;
}