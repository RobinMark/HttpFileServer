#include <stdio.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <fstream>
#include "FileInfo.h"

#pragma comment (lib, "ws2_32.lib")

#define DEFAULT_PORT "8080" 


int handleRequest(SOCKET clientSocket);
int recvUntil(SOCKET sock, char *buf, int lenbuf, const char endstr[]);
int generateResponse(char *method, char *path, char *httpVer, char *headers, char *response, int lenbuf);

int main()
{
	int result = 0;
	
	// 初始化 winsock
	WSADATA wsaData;
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		printf("WSAStartup failed: %d\n", result);
		return 1;
	}
	// 初始化addrinfo
	ADDRINFO *addr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	// 设置IP和端口
	result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addr);
	if (result != 0)
	{
		printf("getaddrinfo failed: %d\n", result);
		WSACleanup();
		return 1;
	}
	SOCKET listenSocket = INVALID_SOCKET;
	// 新建一个socket
	listenSocket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %d\n", WSAGetLastError());
		WSACleanup();
		freeaddrinfo(addr);
		return 1;
	}
	// 绑定
	result = bind(listenSocket, addr->ai_addr, addr->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		printf("bind failed: %d\n", WSAGetLastError());
		WSACleanup();
		freeaddrinfo(addr);
		closesocket(listenSocket);
		return 1;
	}
	freeaddrinfo(addr);

	// 监听
	result = listen(listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		printf("listen failed: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	// 循环获取等待链接
	while (true)
	{
		SOCKET clientSocket = INVALID_SOCKET;
		clientSocket = accept(listenSocket, NULL, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("accept failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
		result = handleRequest(clientSocket);
		if (result != 0)
		{
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
	}
}

int handleRequest(SOCKET clientSocket)
{
	char recvbuf[4096], method[10], path[4096], httpVer[10], headers[4096], response[4096 * 2];
	int result, sendResult;
	printf("++++++++ 有新连接\n");

	result = recvUntil(clientSocket, recvbuf, sizeof(recvbuf) - 1, "\r\n");
	if (result < 0)
	{
		printf("recv failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		return result;
	}
	if (result > 2)
	{
		recvbuf[result - 2] = '\0';
		sscanf(recvbuf, "%s %s %s", method, path, httpVer);

		result = recvUntil(clientSocket, headers, sizeof(headers) - 1, "\r\n\r\n");
		if (result < 0)
		{
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			return result;
		}
		headers[result - 4] = '\0';
		int genRespResult = generateResponse(method, path, httpVer, headers, response, sizeof(response));

		sendResult = send(clientSocket, response, strlen(response), 0);
		if (sendResult == SOCKET_ERROR)
		{
			printf("send failed: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			return sendResult;
		}
		printf("<<< %s", response);
		printf("<<< Bytes sent: %d\n", WSAGetLastError());
		// 如果等于文件或者txt
		if (genRespResult == -10 || genRespResult == -11)
		{
			wstring goalpath;
			wstring url;
			char2wstring(path, url);
			analyzeHtmlPath(url, goalpath);
			TCHAR * pc = wstring2ptchar(goalpath);
			fstream filein;
			filein.open(pc, ios::binary | ios::in);
			char filebuffer[1024];
			do {
				filein.read(filebuffer, 1024);

				sendResult = send(clientSocket, filebuffer, filein.gcount(), 0);
				if (sendResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(clientSocket);
					return sendResult;
				}
			} while (filein.gcount() == 1024);

		}
	}
	result = shutdown(clientSocket, SD_SEND);
	if (result == SOCKET_ERROR)
	{
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		return sendResult;
	}
	closesocket(clientSocket);
	printf("--- Connection closed\n\n");
	return 0;
}

int recvUntil(SOCKET sock, char *buf, int lenbuf, const char endstr[])
{
	char c;
	
	int i = 0, result, str_len = strlen(endstr), str_i = 0;

	while (true)
	{
		result = recv(sock, &c, 1, 0);
		
		if (result < 0)
			return result; //error
		else if (result == 0)
			return i; // 对方关闭了链接
		if (i < lenbuf)
		{
			buf[i] = c;
			i++;
		}
		if (c == endstr[str_i])
		{
			str_i++;
			if (str_i == str_len)
				return i;
		}
		else
			str_i = 0;
	}
}

int generateResponse(char *method, char *path, char *httpVer, char *headers, char *response, int lenbuf)
{
	_snprintf(response, lenbuf, "HTTP/1.0 200 OK\r\n"
		"Content-Type: text/html charset=UTF-8\r\n\r\n"
		"<h2>Hello!</h2> You are visiting %s using %s with %s. Your headers are <pre>%s</pre> <p>Available Drives(Folders):</p>", path, method, httpVer, headers);
	// 如果是根目录就是第一次访问 初始化
	if (strlen(path) == 1)
	{
		vector<FileInfo> drives;
		getDriveLetters(drives); //检测所有盘符
		wstring drivesstr;
		genDriveInfoHtmlStr(drives, drivesstr); //在网页上显示盘符
		char *pDrivesstr = wstring2pchar(drivesstr);
		memcpy(response + strlen(response), pDrivesstr, strlen(pDrivesstr) + 1);
		return 0;
	}
	wstring goalpath;
	wstring url;
	char2wstring(path, url);
	if (analyzeHtmlPath(url, goalpath) != 404)
	{
		vector<FileInfo> files;
		int isfile = getfolders(goalpath, files);
		if (isfile == -10) {
			_snprintf(response, lenbuf, "HTTP/1.0 200 OK\r\n"
				"Content-Type: application/octet-stream\r\n\r\n");
			return -10;  //deal send outside this function 
		}
		else if (isfile == -11) {
			_snprintf(response, lenbuf, "HTTP/1.0 200 OK\r\n"
				"Content-Type: text/plain \r\n\r\n");
			return -11;  //deal send outside this function 
		}
		wstring filestr;
		genFileInfoHtmlStr(files, filestr);
		char *pfilestr = wstring2pchar(filestr);
		memcpy(response + strlen(response), pfilestr, strlen(pfilestr) + 1);
		return 0;
	}
	else 
	{
		printf("404!");
		return 404;
	}

	return 0;
}