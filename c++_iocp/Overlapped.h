#pragma once

#include<WinSock2.h>
#include<Windows.h>
#include<Mswsock.h>

#include"Connection.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")


class Connection;




struct OverLapped {
	enum Type {
		Accept_type,
		Read_type,
		Write_type,
		Connect_type
	};

	//必须是第一个位置
	WSAOVERLAPPED overlapped;
	Type type;  //类型
	WSABUF wsa_buf;  //其实内置 len  +  buff  : 即指向对应 接受/发送  数据的内存块的 地址 and 长度 
	Connection* conn_;
};



