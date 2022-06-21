#include<iostream>
#include<string>

//这两个要注意顺序
#include<WinSock2.h>
#include<Windows.h>
#include<Mswsock.h>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")


#include"myasio.h"



int main() {
	myasio server;
	server.run("127.0.0.1", 9999);


	return 0;
}