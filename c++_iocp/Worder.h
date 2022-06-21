#pragma once
#include<WinSock2.h>
#include<Windows.h>
#include<Mswsock.h>
#include<thread>
#include"Connection.h"
#include"iocpserver.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

class iocpserver;


class Workers {

public:
	Workers(iocpserver* t) :server(t) {}
	

	void workthread();


	void Start();

	iocpserver* server;
	
};