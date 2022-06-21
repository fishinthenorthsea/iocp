#pragma once
#include<WinSock2.h>
#include<Windows.h>
#include<Mswsock.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")
#include <Ws2tcpip.h>

#include<iostream>
#include<string>

#include"CallBack.h"
#include"Connection.h"

#include"chrono.h"

#include"Worder.h"



class iocpserver :public CallExecutor {
public:
	iocpserver() {
		client_count_ = 0;
		msg_count_ = 0;
		socket_ = INVALID_SOCKET;
	}

	void run(const char* ip, unsigned short port);

	int init(const char* ip, unsigned short port, unsigned int nlisten);
	


	int winsockinit();  //打开网络库
	int InitSocket();
	int bind(const char* ip, unsigned short prot);
	int listen(size_t nlisten);
	int Accept();

	void mainloop();


	void AsyncRead(const Connection* conn);
	void AsyncWrite(const Connection* conn, void* data, size_t size);





	HANDLE completion_port_;    // 开放端口
	SOCKET socket_;             //监听


	chronoTimer chrono_time_;         //计时器
	std::atomic<int>client_count_;    //客户端数量
	std::atomic<int>msg_count_;      //1s内获得的消息数量
};