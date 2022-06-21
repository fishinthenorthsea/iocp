#include"Worder.h"


void Workers::workthread() {
	DWORD bytes_transferred;
	ULONG_PTR completion_key;
	DWORD flags = 0;
	OverLapped* over = nullptr;

	while (true) {
		/*
		completion_port     ��ɶ˿ھ��
		bytes_transferred   ������ֽ���
		completion_key
		*/

		bool bret = GetQueuedCompletionStatus(server->completion_port_, &bytes_transferred, &completion_key, reinterpret_cast<LPOVERLAPPED*>(&over), INFINITE);
		if (bret == false) {
			//�ͻ����˳���û�е��� closesocket �����ر�����
			if (GetLastError() == WAIT_TIMEOUT || GetLastError() == ERROR_NETNAME_DELETED) {
			//	//fprintf(stderr, "client:%d �Ͽ�\n",over->conn_->GetSocket());
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
			//���е�ǰ�ᶼҪ������GetQueuedCompletionStatus �ɹ����صĻ�����
			//�����¼�
			if (over->type == OverLapped::Accept_type) {
				if (server->onconnectd_) {
					server->onconnectd_(over->conn_);
				}
				continue;
			}
			//˳�����Ҫ����Ϊaccept Ҳ�� bytes_transferred ==0 �ģ�����Ҫ���ж��ǲ��� accept
			if (bytes_transferred == 0) {
				//�ͻ��˶Ͽ�----->����
				if (server->ondisconnectd_) {
					server->ondisconnectd_(over->conn_);
				}
				closesocket(over->conn_->GetSocket());
				delete over->conn_;
				continue;
			}

			//���첽���Ѿ���
			if (over->type == OverLapped::Read_type) {
				if (server->onread_) {
					server->onread_(over->conn_, reinterpret_cast<char*>(over->conn_->GetReadBuffer()), bytes_transferred);
				}
				continue;
			}


			//���첽д�Ѿ����
			if (over->type == OverLapped::Write_type) {

				auto conn = over->conn_;
				//����һ���ѷ����ֽ���
				conn->Setsend_byte(conn->Getsend_byte() + bytes_transferred);


				//û�з�����
				if (conn->Getsend_byte() < conn->Getsend_all()) {
					//ʣ�෢���ֽ���
					over->wsa_buf.len = conn->Getsend_all() - conn->Getsend_byte();
					//conn->GetWriteBuffer() ��ԭʼ��ַָ�� + �ѷ��͵�ƫ��
					over->wsa_buf.buf = reinterpret_cast<char*>(conn->GetWriteBuffer()) + conn->Getsend_byte();


					DWORD bytes;  //˵ʵ�����о����Ҳ�ò�����
					auto send_result = WSASend(conn->GetSocket(),
						&over->wsa_buf, 1,
						&bytes, 0,
						reinterpret_cast<LPWSAOVERLAPPED>(over),
						NULL
					);


					if (!(send_result == 0 || (send_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))) {
						fprintf(stderr, "WSAsend ��������  ʧ��\n");
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