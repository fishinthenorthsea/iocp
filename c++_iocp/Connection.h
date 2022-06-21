#pragma once
#include<WinSock2.h>
#include<Windows.h>
#include<Mswsock.h>
#include<iostream>

#include"Overlapped.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

#define BUFFSIZE 1024


class Connection {
public:

	Connection(const SOCKET socket);
	




	SOCKET GetSocket() const { return accept_socket_; }

	void* GetReadBuffer()  { return read_buff_; }

	size_t GetReadBufferSize() { return BUFFSIZE; }

	size_t Getsend_byte() { return send_byte; }
	size_t Getsend_all() { return send_all; }

	void* GetWriteBuffer() { return write_buffer_.get(); }


	OverLapped* GetAcceptOverLapped() { return accept_overlapped_.get(); }
	OverLapped* GetWriteOverLapped() { return write_overlapped_.get(); }
	OverLapped* GetReadOverLapped() { return read_overlapped_.get(); }




	size_t GetWriteBufferSize() { return write_size; }
	void SetWriteBufferSize(size_t s) {
		write_size = s;
		write_buffer_.reset(new char[s]);
	}



	void Setsend_byte(size_t t) { send_byte = t; }
	void Setsend_all(size_t t) { send_all = t; }

	SOCKET accept_socket_;

	size_t write_size;  //写缓冲区的长度
	std::unique_ptr<char>write_buffer_;
	size_t send_byte;  //已发送字节数
	size_t send_all;   //要发送的总字节数

	char read_buff_[BUFFSIZE];
	size_t read_cnt;
	char msg_buff_[BUFFSIZE * 4]; //？


	std::unique_ptr<OverLapped> connect_overlapped_;
	std::unique_ptr<OverLapped> accept_overlapped_;
	std::unique_ptr<OverLapped> read_overlapped_;
	std::unique_ptr<OverLapped> write_overlapped_;

};