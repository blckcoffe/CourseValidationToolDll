#pragma once
#include <direct.h>
#include "common.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#include <time.h>
#include <locale>

using std::string;
using std::ifstream;

class fileProcessor
{
private:
	unsigned int videoNumberInFolder;
	unsigned int videoNumberInJson;
	vector<string> vVideosInFolder;
	string sFilePath;
	string sFileName;
	string sFileFolderName;
	string sNoteTitle;
	vector<string> vLinkedVideos;
	vector<string> vCoursewareNames;
	void setFileFolderName(string sName);
	void setFileName(string sFilePath);
	bool processJsonFile();
	bool getCourseNameInSection(string s, string &subString, string stitle);
	bool validate();
public:
	void setFilePath(string sPath);
	void setNoteTitle(string sTitle);
	bool fileValidation();
	fileProcessor();
	fileProcessor(const string fileFolder);
	~fileProcessor();
};

