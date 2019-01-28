#include "FileInfo.h"



FileInfo::FileInfo()
{

}


FileInfo::~FileInfo()
{
}

int FileInfo::setFullHtmlUri()
{
	wchar_t slash = '/';
	wchar_t antislash = '\\';
	fullHtmlUri = wstring(L"");

	for (auto it = fullpath.begin(); it != fullpath.end(); ++it) {

		if ((*it) == antislash) {
			fullHtmlUri.append(L"/");
		}
		else if ((*it) == ' ') {
			fullHtmlUri.append(L"%20");
		}
		else {
			fullHtmlUri.append(1, (*it));
		}
	}
	fullHtmlUri = wstring(L"/files/") + fullpath[0] + fullHtmlUri.substr(2, fullHtmlUri.length() - 2);

	return 0;
}

char *wstring2pchar(const wstring &str)
{
	char *binaryText;
	size_t lenText;
	lenText = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
	binaryText = new char[lenText + 1];
	memset((void*)binaryText, 0, sizeof(char) * (lenText + 1));
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, binaryText, lenText, NULL, NULL);
	return binaryText;
}

// 务必需要delete
wchar_t * wstring2ptchar(const wstring & tstr)
{
	wchar_t * result = new wchar_t [tstr.length() + 1];
	memset(result, 0, sizeof(wchar_t) * (tstr.length() + 1)); 
	for (unsigned int i = 0; i < tstr.length(); ++i) {
		result[i] = tstr[i];
	}
	return result;
}

int char2wstring(char *str, wstring &outstr)
{
	size_t len ;
	size_t converted = 0;
	wchar_t * wstr;
	len = ::MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, NULL);

	wstr = new wchar_t[len];
	memset(wstr, 0, len);
	::MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len);

	outstr = wstring(wstr);
	delete[] wstr;
	return 0;
}

wstring replaceAll(wstring & str, wstring strfind, wstring yourstr)
{
	wstring result;
	bool isSame = false;
	wstring::iterator itf;
	for (auto it = str.begin(); it != str.end(); it++)
	{
		isSame = true;
		int i = 0;
		for (itf = strfind.begin(); itf + i != strfind.end(); i++)
		{
			if ((*(it + i)) != (*(itf + i)))
			{
				isSame = false;
				break;
			}
		}
		if (isSame)
		{
			result.append(yourstr);
			it = it + strfind.length() - 1;
		}
		else
		{
			result.append(1, (*it));
		}
	}
	return result;
}

wstring decode2zhcn(wstring str)
{
	wstring result;
	for (auto it = str.begin(); it != str.end(); it++)
	{
		BYTE temp[1024] = { 0 };
		int i = 0;
		while (it != str.end() && (*it) == '%')
		{
			temp[i] = hex2char(*(it + 1), *(it + 2));
			it = it + 3;
			i++;
		}
		if (i > 0)
		{
			wstring wstr;
			char2wstring((char*)temp, wstr);
			result.append(wstr);
		}
		if (it != str.end())
		{
			result.append(1, (*it));
		}
		if (it == str.end())
		{
			break;
		}
	}
	return result;
}

int hex2char(wchar_t hexa, wchar_t hexb)
{
	return getoffset(hexa) * 16 + getoffset(hexb);
}

int getoffset(wchar_t hex) 
{
	if (hex >= 'a' && hex <= 'z') 
	{
		return (hex - 'a' + 10);
	}
	else if (hex >= 'A' && hex <= 'Z') 
	{
		return (hex - 'A' + 10);
	}
	else if (hex >= '0' && hex <= '9') 
	{
		return (hex - '0');
	}
	else {
		return -1; //error
	}
}


int getDriveLetters(vector<FileInfo> &files)
{
	FileInfo temp;
	wchar_t path[10] = { 0 };
	path[1] = ':';
	path[2] = '\\';
	UINT type;
	for (char c = 'A'; c <= 'Z'; c++)
	{
		path[0] = c;
		type = GetDriveType(path);
		if (type == DRIVE_FIXED)
		{
			temp.name[0] = c;
			files.push_back(temp);
		}
	}
	return 0;
}

int genDriveInfoHtmlStr(vector<FileInfo> &files, wstring &str)
{
	if (files.size() == 0)
	{
		str = str + wstring(L"<p>No files or folders here!</p><p><a href = ""/..""><span class = ""filename"">..</span></a></p>");
		return 2; // 没有 文件
	}

	for (vector<FileInfo>::iterator it = files.begin(); it != files.end(); it++)
	{
		str = str + wstring(L"<p class = ""item"">")
			+ wstring(L"<a href = ""/files/")
			+ wstring((*it).name) //path
			+ wstring(L">")
			+ wstring(L"<span class = ""filename"">")
			+ wstring((*it).name)
			+ wstring(L"</span></a></p>");
	}
	return 0;
}

int genFileInfoHtmlStr(vector<FileInfo>& files, wstring & str)
{
	if (files.size() == 0) 
	{
		str = str = wstring(L"<p>No files or folders here!< / p><p><a href = "" / ..""><span class = ""filename"">..< / span>< / a>< / p>");
		return 2;
	}
	for (vector<FileInfo>::iterator it = files.begin(); it != files.end(); it++) {


		str = str + wstring(TEXT("<p class = ""item"">"))
			+ wstring(TEXT("<a href = """))
			+ wstring((*it).fullHtmlUri)   //URL
			+ wstring(TEXT(""">"))
			+ wstring(TEXT("<span class = ""filename"">"))  // ???????  ???? ?????????
			+ wstring((*it).name)
			+ wstring(TEXT("</span></a></p>"));
	}
	return 0;
}

int analyzeHtmlPath(wstring url, wstring & goalPath)
{
	/*
	if (url[url.length() - 1] == '/') {
		url.resize(url.length() - 1);
	}
	*/
	wchar_t endchar = '.';
	wchar_t slash = '/';
	wchar_t antislash = '\\';
	wstring realpath;
	size_t kpos;
	wchar_t keyword[] = L"/files/";

	kpos = url.find(keyword, 0);
	if (kpos == wstring::npos)
	{
		return 404; //找不到files 说明不是用户指定的路径 可能是访问网站图片之类的
	}
	size_t startCopyOffSet = kpos + wcslen(keyword) + 1;
	// 如果末尾是. 或者 ..
	if (url[url.length() - 1] == endchar)
	{
		// 待处理
		/*
		if (url[url.length() - 2] == endchar) 
		{
			int pos = url.rfind(slash, url.length() - 4);  //  /..  3 charactors
			realpath.append(url, startCopyOffSet, pos - startCopyOffSet);
		}
		else 
		{
			realpath.append(url, startCopyOffSet, url.length() - 2 - startCopyOffSet);
		}
		*/
	}
	else
	{
		realpath.assign(url.begin() + startCopyOffSet, url.end());
	}

	realpath = replaceAll(realpath, wstring(L"%20"), wstring(L" "));
	for (auto it = realpath.begin(); it != realpath.end(); it++)
	{
		// 如果路径中是斜杠就替换成反斜杠
		if ((*it) == slash)
		{
			(*it) = antislash;
		}
	}
	realpath = decode2zhcn(realpath);

	goalPath = url[kpos + wcslen(keyword)] + wstring(L":") + realpath;
	
	return 0;
}

int getfolders(wstring goalpath, vector<FileInfo>& files)
{
	wchar_t *temp = wstring2ptchar(goalpath);
	int itemp = getfolders(temp, files);
	delete[] temp;

	return itemp;
}

int getfolders(wchar_t * goalpath, vector<FileInfo>& files)
{
	size_t pathlen;
	StringCchLength(goalpath, MAX_PATH, &pathlen);
	if (pathlen > (MAX_PATH - 3)) {
		wprintf(L"ERROR: Directory path is too long! \n");
		return (-1);
	}

	wchar_t path[MAX_PATH];
	WIN32_FIND_DATA ffd;
	// Prepare path for Findfile
	StringCchCopy(path, MAX_PATH, goalpath);
	StringCchCat(path, MAX_PATH, L"\\*");
	HANDLE hFind = FindFirstFile(path, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		wprintf(L"findfirstfile error!");
		if (wcscmp(path + wcslen(path) - 4, L".txt") == 0) {
			return -11;
		}
		else {
			return -10;
		}
	}

	FileInfo temp;
	StringCchCopy(temp.name, MAX_PATH, ffd.cFileName);
	temp.fd = ffd;
	temp.fullpath = wstring(goalpath) + L"\\" + wstring(temp.name); 
	temp.setFullHtmlUri();
	files.push_back(temp);

	while (FindNextFile(hFind, &ffd) != 0) {
		StringCchCopy(temp.name, MAX_PATH, ffd.cFileName);
		temp.fd = ffd;
		temp.fullpath = wstring(goalpath) + L"\\" + wstring(temp.name); 
		temp.setFullHtmlUri();
		files.push_back(temp);
	}

	FindClose(hFind);

	if (files.size() == 0) {
		return (-1); 
	}
	return GetLastError();
}

