#include "SocketComm.h"
using namespace std;

SocketComm::SocketComm() {
	// set default buffer size as 2
	bufferSize = 2;
	buf = new char[bufferSize+1];
}

SocketComm::SocketComm(int bufSize) {
	bufferSize = bufSize;
	buf = new char[bufferSize];
}

SocketComm::~SocketComm() {
	delete buf;
}

int SocketComm::Init(int portNum) {
	// initialize winsocket 
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		Err_display("WSAStartup()");
		return 0;
	}

	// set socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (client_sock == INVALID_SOCKET) {
		Err_display("socket()");
		return 0;
	}

	char addr[INET_ADDRSTRLEN] = "127.0.0.1";

	// initialize client addr
	ZeroMemory(&clientaddr, sizeof(clientaddr));
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr(addr);
	clientaddr.sin_port = ntohs(portNum);
	memset(&(clientaddr.sin_zero), '\0', 8);
	
	printf("Connecting to %s ...\n", addr);
	if (connect(client_sock, (struct sockaddr*)&clientaddr, sizeof(clientaddr)) < 0) {
		Err_display("connect()");
		return 0;
	}
	printf("Successfully connected to %s\n", addr);
	SendDatatoServer();
	printf("Sended id to server\n");
	
	return 1;
}

void SocketComm::Err_display(char* msg) {
	printf("[Socket] %s", msg);
}

int SocketComm::SendDatatoServer() {
	char name[] = "Haptic Application\n";
	retval = send(client_sock, name, sizeof(name), 0);
	if (retval == SOCKET_ERROR) {
		Err_display("send");
		return 0;
	}
	
	return 1;
}

int SocketComm::ReceiveDatafromServer() {
	// receive data from server
	retval = recv(client_sock, buf, sizeof(bufferSize), 0);
	if (retval == SOCKET_ERROR) {
		Err_display("recv");
		return 0;
	}
	buf[retval] = '\0';

	return 1;
}

void SocketComm::PrintData() {
	printf("Received from Client : %s\n", buf);
}

char SocketComm::GetSerialData() {
	return buf[0];
}

void SocketComm::Close() {
	closesocket(client_sock);
	WSACleanup();
}