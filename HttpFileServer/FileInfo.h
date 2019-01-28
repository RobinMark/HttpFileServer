#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <strsafe.h>

using namespace std;

class FileInfo
{
public:
	FileInfo();
	~FileInfo();

	wchar_t name[MAX_PATH] = { 0 };
	wstring fullHtmlUri;
	wstring fullpath;
	WIN32_FIND_DATA fd;

	int setFullHtmlUri();
};

char *wstring2pchar(const wstring &str);
wchar_t *wstring2ptchar(const wstring &tstr);
int char2wstring(char *str, wstring &outstr);
wstring replaceAll(wstring &str, wstring strfind, wstring yourstr);
wstring decode2zhcn(wstring str);


int hex2char(wchar_t hexa, wchar_t hexb);
int getoffset(wchar_t hex);

int getDriveLetters(vector<FileInfo> &files);
int genDriveInfoHtmlStr(vector<FileInfo> &files, wstring &str);
int genFileInfoHtmlStr(vector<FileInfo> &files, wstring &str);
int analyzeHtmlPath(wstring url, wstring &goalPath);
int getfolders(wstring goalpath, vector<FileInfo> & files);
int getfolders(wchar_t *goalpath, vector<FileInfo> & files);