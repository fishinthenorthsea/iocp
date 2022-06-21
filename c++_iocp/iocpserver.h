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
	


	int winsockinit();  //�������
	int InitSocket();
	int bind(const char* ip, unsigned short prot);
	int listen(size_t nlisten);
	int Accept();

	void mainloop();


	void AsyncRead(const Connection* conn);
	void AsyncWrite(const Connection* conn, void* data, size_t size);





	HANDLE completion_port_;    // ���Ŷ˿�
	SOCKET socket_;             //����


	chronoTimer chrono_time_;         //��ʱ��
	std::atomic<int>client_count_;    //�ͻ�������
	std::atomic<int>msg_count_;      //1s�ڻ�õ���Ϣ����
};