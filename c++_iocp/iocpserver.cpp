#include"iocpserver.h"
#include"Overlapped.h"


void iocpserver::run(const char* ip, unsigned short port) {
	//��ʼ��һ��iocp������
	if (init(ip, port, 10000) == -1) {
		fprintf(stderr, "iocp����������ʧ��\n");
		return;
	}

	mainloop();
}


//֮��Ӧ�øĳɶ��߳�
void iocpserver::mainloop() {
	while (1) {
		Sleep(1000);
		std::cout << "client:<" << client_count_ << "> msg:<"<<msg_count_<<">\n";
		msg_count_ = 0;
		chrono_time_.FlushTime();
	}
}








//�������/���������
int iocpserver::winsockinit() {
	int ret = -1;
	do {
		WORD version = MAKEWORD(2, 2);
		WSADATA wsadata;
		//�ɹ�����0
		//����
		auto _wsa_inited = !WSAStartup(version, &wsadata);
		if (!_wsa_inited)
			break;
		if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2) {
			WSACleanup();
			_wsa_inited = false;
			break;
		}
		ret = 0;
	} while (0);
	return ret;
}





//���� ����socket ���ҹ��� ��ɶ˿�
int iocpserver::InitSocket() {
	int ret = 0;
	do {
		//ע�������WSA_FLAG_OVERLAPPED   IOCP ģ�� ר��
		socket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (socket_ == INVALID_SOCKET) {
			fprintf(stderr, "���� WSASocket ʧ��\n");
			ret = -1;
			break;
		}
		//���´����ļ���socket ��֮ǰ����ɶ˿� �������� ����
		if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket_), completion_port_, 0, 0)) {
		}
	} while (0);

	return ret;
}



//Ϊ����socket ��
int iocpserver::bind(const char* ip, unsigned short prot) {
	SOCKADDR_IN addr;

	//addr.sin_addr.S_un.S_addr = inet_addr(ip);
	inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(prot);


	if (::bind(socket_, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		fprintf(stderr, "bind  ʧ��\n");
		return -1;
	}
	return 0;
}


//������Ǽ������еĳ���
int iocpserver::listen(size_t nlisten) {
	if (::listen(socket_, nlisten) == SOCKET_ERROR) {
		fprintf(stderr, "listen  ʧ��\n");
		return -1;
	}
	return 0;
}




int iocpserver::Accept() {
	int ret = -1;
	do {
		/*
		���ﲻ���� WSAAccept
		��Ϊ��Ҫ�ڲ�ѯ��ɶ˿ڵ�ʱ���жϳ�
		����Ϣ������
		��������չ��
		*/

		//acceptEx ������ָ��
		//fprintf(stderr,"listen\n");
		LPFN_ACCEPTEX accept_func;
		GUID accept_guild = WSAID_ACCEPTEX;  //�̶���guid



		DWORD bytes;

		//��ȡ����LPFN_ACCEPTEX �ĺ���ָ��--->��accept_func��ֵ
		ret = WSAIoctl(
			socket_, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&accept_guild, sizeof(accept_guild),
			&accept_func, sizeof(accept_func),
			&bytes, NULL, NULL
		);
		if (ret != 0) {
			ret = -1;
			fprintf(stderr, "��ȡaccpet ������ַ ʧ��\n");
			break;
		}



		//����һ���������ӵ� socket
		//��ͨ��accept�ǵ��ú�᷵��һ�����ܵ���socket
		//�����첽��һ������Ҫ���ֶ�����һ����Ȼ��󶨹�ȥ
		SOCKET accept_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (accept_socket == SOCKET_ERROR) {
			fprintf(stderr, "��ʼ��accept socket  ʧ��\n");
			ret = -1;
			break;
		}



		//���������ͣ���ʱ�򼴿�����ɶ˿ڴ��������øú���ȥ��ͣ
		//��ȥҪ���ȡ������
		//so all �첽����������Ҫ��ͣ��ȥ����-----> �Ӷ�����ȡ��ʱ�������ٴε���
		
		//���������ֽṹ��Ӧ�ñ��Ż�
		//ʹ�����ӳ�
		//��Ϊ������ʹ��new ���ᵼ�´������ڴ���Ƭ
		//����Ӧ���������ӳ� -----------> asio �����������ӳ�
		std::unique_ptr<Connection> conn = std::make_unique<Connection>(accept_socket);
		

		
		//memset(accept_over.get(), 0, sizeof(accept_over));
		//accept_over->type = OverLapped::Accpet_type;


		DWORD n = 0;    //��¼һ��ʵ���յ��ĳ���
		//�ص�������һ������---->�ص��ṹ��ָ��
		//��ʵ�����Լ���ƵĽṹ��
		//֮�����е����ã�����������Ӧ���ɸýṹ���ṩ
		const int accept_results = accept_func(
			socket_,                  //����socket
			accept_socket,            //����socket--->�����´�����socket
			conn->GetReadBuffer(),    //���ݻ�����
			0,                        //�����ȡ�����ݴ�С    0--> ��accpet ����ֻ��Ϊ�˽������ӣ����������ݵĶ�д
			sizeof(sockaddr_in) + 16,   //���ص�ַ�ĳ���  
			sizeof(sockaddr_in) + 16,   //Զ�̵�ַ�ĳ���
			&n,                         //ʵ���յ��ĳ���
			//ע��������˸�ǿת����Ϊ����������LPOVERLAPPED
			//����ֻҪOverLapped�ṹ��ĵ�һ����Ա������LPOVERLAPPED
			//���ɽ���ǿת������
			reinterpret_cast<LPOVERLAPPED>(conn->GetAcceptOverLapped())
		);

		if (!(accept_results == true || WSAGetLastError() == WSA_IO_PENDING)) {
			ret = -1;
			fprintf(stderr, "����accept ex  ʧ��\n");
			break;
		}



		//��accept_socket ������ ��ɶ˿�
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(accept_socket), completion_port_, 0, 0);
		//�������ָ��Ĺ���Ȩ�ޣ��ö���Ӧ�û���ȥ
		conn.release();

	} while (0);
	return ret;
}




int iocpserver::init(const char* ip, unsigned short port, unsigned int nlisten) {
	int ret = 0;
	do {
		ret = winsockinit();   //�������
		if (ret == -1) {
			fprintf(stderr, "��ʼ�� winsock init ʧ��\n");
			break;
		}

		//������ɶ˿�
		completion_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (!completion_port_) {
			fprintf(stderr, "������ɶ˿� ʧ��\n");
			ret = -1;
			break;
		}

		//��ʼ������socket
		if ((ret = InitSocket()) == -1) {
			break;
		}

		//�󶨶˿�
		if ((ret = bind(ip, port)) == -1) {
			break;
		}

		if ((ret = listen(nlisten)) == -1) {
			break;
		}



		Workers* work = new Workers(this);
		work->Start();


		/*
		if (Accept() == -1) {
			break;
		}
		*/
	} while (0);

	return ret;
}


void iocpserver::AsyncWrite(const Connection* conn, void* data, size_t size) {
	auto con = const_cast<Connection*>(conn);
	//����д�뻺������С
	if (con->GetWriteBufferSize() < size) {
		con->SetWriteBufferSize(size);
	}

	memcpy_s(con->GetWriteBuffer(), con->GetWriteBufferSize(),data ,size);

	con->Setsend_byte(0);
	con->Setsend_all(size);

	auto over = con->GetWriteOverLapped();
	over->wsa_buf.len = size;
	over->wsa_buf.buf = static_cast<char*>(con->GetWriteBuffer());

	DWORD bytes;
	auto send_result = WSASend(con->GetSocket(),
		&over->wsa_buf, 1,
		&bytes, 0,
		reinterpret_cast<LPWSAOVERLAPPED>(over),
		NULL
	);

	if (!(send_result == 0 || (send_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))) {
		fprintf(stderr, "WSAsend ��������  ʧ��\n");
	}
}



void iocpserver::AsyncRead(const Connection* conn) {
	auto con = const_cast<Connection*>(conn);
	auto over = con->GetReadOverLapped();

	over->wsa_buf.buf = static_cast<char*>(over->conn_->GetReadBuffer());
	over->wsa_buf.len = over->conn_->GetReadBufferSize();

	DWORD flags = 0;
	DWORD bytes_transferred = 0;  //������ֽ���

	const int recv_result = WSARecv(over->conn_->GetSocket(),
		&over->wsa_buf, 1, &bytes_transferred, &flags,
		reinterpret_cast<LPWSAOVERLAPPED>(over), NULL);

	if (!(recv_result == 0 || (recv_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))) {
		fprintf(stderr, "WSARecv ��������  ʧ��\n");
	}
}