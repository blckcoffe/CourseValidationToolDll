// JsonErrorCheck.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <cstdio>
#include "rapidjson/document.h"
#include "fileProcessor.h"
#include <direct.h>
#include <wchar.h>  

#ifdef _WIN32  
#include <Windows.h>  
#endif  



#define psln(x) std::cout << #x " = " << (x) << std::endl
#define LIBEXPORT_API extern "C" _declspec(dllexport)


using namespace rapidjson;
using namespace std;
int main() {


}

LIBEXPORT_API int _stdcall ExecuteValidation( const char* filename) {
	fileProcessor lo_fileProcessor(filename);
	if ( lo_fileProcessor.fileValidation( ) ) {
		return 1;
	}
	else {
		return 0;
	}
}

