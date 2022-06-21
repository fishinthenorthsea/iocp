#pragma once
#include<iostream>
#include<string>
#include"Message.hpp"
//������Ҫע��˳��
#include<WinSock2.h>
#include<Windows.h>
#include<Mswsock.h>
#include<WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

#define BUFFSIZE 10000




class TcpClient {
public:
	TcpClient():socket_(INVALID_SOCKET), pos_(0)
	{
		memset(buf_, 0, sizeof(BUFFSIZE));
		memset(msgbuf_, 0, sizeof(BUFFSIZE*4));
		InitSocket();
	}
	~TcpClient() {
		Close();
	}

	void InitSocket() {
		do {
			//��ʼ�������
			WORD version = MAKEWORD(2, 2);
			WSADATA wsadata;
			WSAStartup(version, &wsadata);

			//ע�������WSA_FLAG_OVERLAPPED   IOCP ģ�� ר��
			socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (socket_ == INVALID_SOCKET) {
				fprintf(stderr, "���� WSASocket ʧ��\n");
				break;
			}
			
		} while (0);

	}

	int Connect(const char* ip, unsigned short port) {
		if (socket_ == INVALID_SOCKET) {
			InitSocket();
		}
		SOCKADDR_IN addr;
		memset(&addr, 0, sizeof(addr));
		//inet_pton(AF_INET, ip, &ADDR.sin_addr);
		//addr.sin_addr.S_un.S_addr = inet_addr(ip);

		inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);

		if (connect(socket_, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
			fprintf(stderr, "connect  ʧ��\n");
			return -1;
		}

	}


	void Close() {
		if (socket_ != INVALID_SOCKET) {
			closesocket(socket_);
			WSACleanup();
		}
	}

	void ProcessMessage(MesageHeader* mh) {
		switch (mh->type_){
		case T_Login_Result:
			fprintf(stderr, "T_Login_Result ���ݳ���: %d\n", mh->len_);
			break;
		case T_Logout_Result:
			fprintf(stderr, "T_Logout_Result ���ݳ���: %d\n", mh->len_);
			break;
		case T_Error:
			fprintf(stderr, "T_Error%d\n");
			break;
		default:
			fprintf(stderr, "δ֪��Ϣ%d\n");
			break;
		}
	}


	int RecvData() {
		/*
		int ret = -1;
		MesageHeader* mh;
		do {
			ret = recv(socket_, buf_, BUFFSIZE, 0);
			if (ret <= 0) {
				closesocket(socket_);
				break;
			}

			memcpy(msgbuf_ + pos_, buf_, ret);
			pos_ += ret;

			while (pos_ >= sizeof(MesageHeader)) {
				mh = (MesageHeader*)msgbuf_;
				//����Ϣ��������
				if (mh->len <= pos_) {
					pos_ -= mh->len;
					ProcessMessage(mh);
					memcpy(msgbuf_, msgbuf_ + mh->len, pos_);
				}
				else
					break;
			}
		} while (0);
		*/

		recv(socket_, buf_, BUFFSIZE, 0);

	}




	void SendData(void* data, size_t len) {
		send(socket_, reinterpret_cast<char*>(data), len, 0);
	}



	int run(const char* ip, unsigned short port) {
		int ret = Connect(ip, port);


		//std::string msg="123132156465654asdasdasdzxc23x1c3zx2c564564as56d4a56s4d6as5d456as4das5d4";
		while (1) {
			std::string msg;
			std::cout << "send: ";
			std::cin >> msg;


			send(socket_, msg.c_str(), msg.size(), 0);


			fd_set fdreads;
			FD_ZERO(&fdreads);
			FD_SET(socket_, &fdreads);

			//����500����ĳ�ʱ
			timeval t = { 0,500 };




			int ret = select(socket_, &fdreads, 0, 0, NULL);
			if (ret < 0) {
				printf("select �������\n");
				break;
			}

			if (FD_ISSET(socket_, &fdreads)) {
				int  n = recv(socket_, buf_, BUFFSIZE, 0);
				if (n <= 0) {
					closesocket(socket_);
					break;
				}
				buf_[n] = '\0';
				fprintf(stderr, "recv: %s\n\n", buf_);

			}


		}


		/*
		Login login;
		strcpy(login.name_, "zwk");
		strcpy(login.passwd_, "123456");


		while (1) {
			if (ret == -1)
				break;
			if (SOCKET_ERROR == SendData(&login))
				break;

			fd_set fdreads;
			FD_ZERO(&fdreads);
			FD_SET(socket_, &fdreads);

			//����500����ĳ�ʱ
			timeval t = { 0,500 };

			int ret = select(socket_, &fdreads, 0, 0, &t);
			if (ret < 0) {
				printf("select �������\n");
				break;
			}

			if (FD_ISSET(socket_, &fdreads)) {
				if(RecvData() <= 0) {
					printf("select�������2\n");
					break;
				}
			}

			//1s ��һ��
			Sleep(1000);

		}

		printf("�������Ͽ�");
		return ret;
		*/
		return 0;
	}

private:
	SOCKET socket_;
	
	char buf_[BUFFSIZE];
	char msgbuf_[BUFFSIZE * 4]; //���ݻ���
	size_t pos_;
};