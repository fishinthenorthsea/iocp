#pragma once


#include"iocpserver.h"
#include<thread>
class myasio {
public:
	//初始化时，就为其绑定默认回调函数
	//允许其自定义回调接口
	myasio() {
		server.setonconnectd([this](const Connection* conn) {OnClientConnected(conn); });
		server.setondisconnectd([this](const Connection* conn) {OnClientdisConnected(conn); });
		server.setonread([this](const Connection* conn, void* data, size_t size) {OnRead(conn,data,size); });
		server.setonwrite([this](const Connection* conn) {OnWrite(conn); });
	}

	void run(const char* ip, unsigned short port);




	void OnClientConnected(const Connection* conn) {
		++server.client_count_;   //客户端++

		//fprintf(stderr, "socket:%d is connect\n", conn->GetSocket());
		server.AsyncRead(conn);
		server.Accept();
	}



	void OnClientdisConnected(const Connection* conn) {
		--server.client_count_;
		//fprintf(stderr, "socket:%d is disconnect\n", conn->GetSocket());
	}


	void OnRead(const Connection* conn, void* data, size_t size)
	{
		//一系列操作后 ----> 异步写
		server.AsyncWrite(conn, data, size);
	}

	void OnWrite(const Connection* conn) {
		++server.msg_count_;
		server.AsyncRead(conn);
	}



private:
	iocpserver server;
};