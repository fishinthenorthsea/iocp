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

	//�����ǵ�һ��λ��
	WSAOVERLAPPED overlapped;
	Type type;  //����
	WSABUF wsa_buf;  //��ʵ���� len  +  buff  : ��ָ���Ӧ ����/����  ���ݵ��ڴ��� ��ַ and ���� 
	Connection* conn_;
};



