#ifndef SOCKET_COMMUNICATION_H
#define SOCKET_COMMUNICATION_H
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma once
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <WS2tcpip.h>
#include <Windows.h>
#pragma comment(lib, "ws2_32")

using namespace std;

class SocketComm {
public:
	SocketComm();
	SocketComm(int bufSize);
	~SocketComm();

	int Init(int portNum);
	void Err_display(char* msg);
	int ReceiveDatafromServer();
	int SendDatatoServer();
	void PrintData();
	char GetSerialData();
	void Close();
private:
	int bufferSize; // buffer size
	int portNumber; // port number
	int retval; // result value
	WSADATA wsa; // data for networking
	SOCKET client_sock; // socket
	SOCKADDR_IN clientaddr; // socket addresses
	char* buf; // buffer
};

#endif

