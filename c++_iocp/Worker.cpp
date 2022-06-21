#include"Worder.h"


void Workers::workthread() {
	DWORD bytes_transferred;
	ULONG_PTR completion_key;
	DWORD flags = 0;
	OverLapped* over = nullptr;

	while (true) {
		/*
		completion_port     完成端口句柄
		bytes_transferred   传输的字节数
		completion_key
		*/

		bool bret = GetQueuedCompletionStatus(server->completion_port_, &bytes_transferred, &completion_key, reinterpret_cast<LPOVERLAPPED*>(&over), INFINITE);
		if (bret == false) {
			//客户端退出，没有调用 closesocket 正常关闭连接
			if (GetLastError() == WAIT_TIMEOUT || GetLastError() == ERROR_NETNAME_DELETED) {
			//	//fprintf(stderr, "client:%d 断开\n",over->conn_->GetSocket());
				if (over != NULL) {
					if (over->conn_ != NULL) {
						if (server->ondisconnectd_) {
							server->ondisconnectd_(over->conn_);
						}
						delete over->conn_;
						continue;
					}
				}
			}
		}
		else
		{
			//所有的前提都要建立在GetQueuedCompletionStatus 成功返回的基础上
			//连接事件
			if (over->type == OverLapped::Accept_type) {
				if (server->onconnectd_) {
					server->onconnectd_(over->conn_);
				}
				continue;
			}
			//顺序很重要，因为accept 也是 bytes_transferred ==0 的，所以要先判断是不是 accept
			if (bytes_transferred == 0) {
				//客户端断开----->正常
				if (server->ondisconnectd_) {
					server->ondisconnectd_(over->conn_);
				}
				closesocket(over->conn_->GetSocket());
				delete over->conn_;
				continue;
			}

			//即异步读已经完
			if (over->type == OverLapped::Read_type) {
				if (server->onread_) {
					server->onread_(over->conn_, reinterpret_cast<char*>(over->conn_->GetReadBuffer()), bytes_transferred);
				}
				continue;
			}


			//即异步写已经完成
			if (over->type == OverLapped::Write_type) {

				auto conn = over->conn_;
				//更新一下已发送字节数
				conn->Setsend_byte(conn->Getsend_byte() + bytes_transferred);


				//没有发送完
				if (conn->Getsend_byte() < conn->Getsend_all()) {
					//剩余发送字节数
					over->wsa_buf.len = conn->Getsend_all() - conn->Getsend_byte();
					//conn->GetWriteBuffer() 即原始地址指针 + 已发送的偏移
					over->wsa_buf.buf = reinterpret_cast<char*>(conn->GetWriteBuffer()) + conn->Getsend_byte();


					DWORD bytes;  //说实话，感觉这个也用不到啊
					auto send_result = WSASend(conn->GetSocket(),
						&over->wsa_buf, 1,
						&bytes, 0,
						reinterpret_cast<LPWSAOVERLAPPED>(over),
						NULL
					);


					if (!(send_result == 0 || (send_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))) {
						fprintf(stderr, "WSAsend 发送数据  失败\n");
					}
				}
				else {
					//AsyncRead(over->conn_);
					if (server->onwrite_) {
						server->onwrite_(over->conn_);
					}
				}
			}

		}
	}
}


void Workers::Start() {
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int g_ThreadCount = sysInfo.dwNumberOfProcessors * 2;


	for (int i = 0; i < g_ThreadCount; ++i) {
		std::thread t([this]() {workthread(); });
		t.detach();
	}

	server->Accept();
}