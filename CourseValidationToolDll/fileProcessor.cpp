#include "stdafx.h"
#include "fileProcessor.h"
#include <io.h>
#include <regex>

void getAllFiles(string path, vector<string>& files, string fileType);

fileProcessor::fileProcessor()
{
	videoNumberInFolder = 0;
	videoNumberInJson = 0;
	sFilePath = "";
	sFileName = "";
	sFileFolderName = "";
	sNoteTitle = "";
}

fileProcessor::fileProcessor(const string fileFolder) {
	if (fileFolder == "")
	{
		return;
	}

	sFilePath = fileFolder;
	setFileName(fileFolder);

	size_t sPos;
	sPos = fileFolder.find_last_of("\\");
	string sFolderName;
	if (sPos != string::npos)
	{
		sFolderName = fileFolder.substr(sPos + 1);
		setFileFolderName(sFolderName);
	}

	getAllFiles(sFilePath, vVideosInFolder, ".mp4");
	videoNumberInFolder = vVideosInFolder.size();
}

fileProcessor::~fileProcessor()
{
}


void fileProcessor::setFilePath(string sPath){
	sFilePath = sPath;
	setFileName(sPath);
	size_t sPos;
	sPos = sPath.find_last_of("\\");
	string sFolderName;
	if (sPos != string::npos )
	{
		sFolderName = sPath.substr(sPos + 1);
		setFileFolderName(sFolderName);
	}

}

void  fileProcessor::setFileName(string sFilePath) {
//获取Json 文件
	vector<string> files;
	getAllFiles(sFilePath, files, ".json");
	string file;

	for (auto iter = files.cbegin(); iter != files.cend(); iter++) {
		file = *iter;
		if (string::npos == file.find("_Extend.json")){
			sFileName = file;
		}
	}
}

void fileProcessor::setNoteTitle(string sTitle){
//	if (sTitle == sFileFolderName)
//	{
		sNoteTitle = sTitle;
//	}
	
}

bool fileProcessor::fileValidation( ){
	if (true == processJsonFile()){
		return validate();
	}
	return false;
}

void fileProcessor::setFileFolderName(string sName){
	sFileFolderName = sName;
}

bool fileProcessor::processJsonFile(){
	// read json content into string.
	string      stringFromStream;
	ifstream    in;
	in.open(sFileName, ifstream::in);
	if (!in.is_open())
		return false;
	string line;
	while (getline(in, line)) {
		stringFromStream.append(line + "\n");
	}
	in.close();

	// ---------------------------- read json --------------------
	// parse json from string.
	using rapidjson::Document;
	Document doc;
	doc.Parse<0>(stringFromStream.c_str());
	if (doc.HasParseError()) {
		rapidjson::ParseErrorCode code = doc.GetParseError();
		return false;
	}

	sNoteTitle = doc["notetitle"].GetString();
	string sVideoName;
	// use values in parse result.
	using rapidjson::Value;
	Value & vLinkedVideo = doc["linkedvideo"];
	if (vLinkedVideo.IsArray()) {
		for (size_t i = 0; i < vLinkedVideo.Size(); ++i) {
			Value & vL = vLinkedVideo[i];
			assert(vL.IsString());
			//			psln(vL.GetString());
			sVideoName = vL.GetString();
			if (string::npos != sVideoName.find(sNoteTitle))
			{
				vLinkedVideos.push_back(vL.GetString());
			}
		}
	}

	Value & vNoteContent = doc["notecontent"];
	string sPlayCoursewareName;

	if (vNoteContent.IsArray()) {
		for (size_t i = 0; i < vNoteContent.Size(); ++i) {
			Value & vN = vNoteContent[i];
			assert(vN.IsObject());
			if (vN.HasMember("sectioncontent") && vN["sectioncontent"].IsArray()) {
				Value &vSectionContent = vN["sectioncontent"];
				assert(vSectionContent.IsArray());
				for (size_t i = 0; i < vSectionContent.Size(); ++i){
					Value &vSC = vSectionContent[i];
					assert(vSC.IsObject());
					if (vSC.HasMember("section") && vSC["section"].IsArray())
					{
						Value &vS = vSC["section"];
						string sSectionValue;
						for (size_t i = 0; i < vS.Size(); ++i){
							assert(vS[i].IsString());
							sSectionValue = vS[i].GetString();
							if (getCourseNameInSection(sSectionValue, sPlayCoursewareName, sNoteTitle)){
								vCoursewareNames.push_back(sPlayCoursewareName);
							}
						}
					}
				}
				videoNumberInJson = vCoursewareNames.size();
				break; //"sectioncontent 节点处理完毕，其它节点不需要处理"
			}
		}
	}

	return true;
}

bool fileProcessor::getCourseNameInSection(string s, string &subString, string stitle){
	size_t iTitleLength;
	size_t iPos;
	iPos = s.find(stitle);
	if (string::npos != iPos)
	{
		iTitleLength = stitle.length();
		subString = s.substr(iPos, iTitleLength + 2);
		regex re0("^" + stitle + "[_][0-9]*");
		if ( regex_match(subString, re0)) {
			return true;
		}
	}

	return false;
}

bool  fileProcessor::validate(){
	string sVideoName;
	bool fFirstElement = true;
	bool fileSuccess = true;
	size_t iPos;

	time_t rawtime;
	time(&rawtime);
	char localtime[64];
	ctime_s(localtime, 64, &rawtime);
	ofstream logInfo( "checklog.txt");
	logInfo << "检查错误开始：" << localtime << endl;
	logInfo << "1：" << "校验文件夹中的视频" << endl;
	logInfo << "*" << "视频个数" << videoNumberInFolder<<endl;

	string videoName;
	string videoNameExp;
	regex re0("^" + sFileFolderName + "[_][0-9]*.mp4");
	for (auto iter = vVideosInFolder.cbegin(); iter != vVideosInFolder.cend(); iter++) {
		iPos = (*iter).find_first_of(sFilePath);
		if ((string::npos == iPos)||( 0 != iPos))
		{
			logInfo << "*Error: " << (*iter) << "不匹配" << endl;
			fileSuccess = false;
		}
		else 
		{
			videoName = (*iter).substr( sFilePath.length()+ 1, string::npos );
			if ( !regex_match(videoName, re0)) {
				logInfo << "**Error: " << (*iter) << "不匹配" << endl;
				fileSuccess = false;
			}
		}
	}

	logInfo << "*：" << "文件夹中的视频名称校验完成" << endl;
	logInfo << "2：" << "======校验Json======" << endl;
	if (videoNumberInJson != videoNumberInFolder ) {
		logInfo << "*Error: " << " Json 中视频个数和文件夹中不匹配 " << endl;
		fileSuccess = false;
	}

	size_t i = 0;
	for (auto iter = vLinkedVideos.cbegin(); iter != vLinkedVideos.cend(); iter++){
		sVideoName = *iter;
		if (fFirstElement)
		{
			fFirstElement = false;
			if  (sVideoName == sNoteTitle + ".obb")
			{				
				continue;
			}
			else{
				logInfo << "*Error: " << sNoteTitle + ".obb" << "没有找到" << endl;
				fileSuccess = false;
			}
		}
		
		iPos = sVideoName.find(".");
		if ( string::npos != iPos )
		{
			sVideoName = sVideoName.substr(0, iPos);
		}

		if (( videoNumberInJson > i )&&( sVideoName != vCoursewareNames[i++] ))
		{
			logInfo << "*Error: " << sVideoName << "  没有找到" << endl;
			fileSuccess = false;
		}
	}

	if (fileSuccess )
	{
		logInfo << "没发现什么问题" << endl;
	}
	else
	{
		logInfo << "*Error: " << "检查文件失败" << endl;
		logInfo.close();
		return false;
	}

	logInfo.close();
	return true;
}


void getAllFiles(string path, vector<string>& files, string fileType)
{
	// 文件句柄
	long hFile = 0;
	// 文件信息
	struct _finddata_t fileinfo;

	string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*" + fileType).c_str(), &fileinfo)) != -1) {
		do {
			// 保存文件的全路径
			files.push_back(p.assign(path).append("\\").append(fileinfo.name));

		} while (_findnext(hFile, &fileinfo) == 0); //寻找下一个，成功返回0，否则-1

		_findclose(hFile);
	}
}