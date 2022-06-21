#include"iocpserver.h"
#include"Overlapped.h"


void iocpserver::run(const char* ip, unsigned short port) {
	//初始化一下iocp服务器
	if (init(ip, port, 10000) == -1) {
		fprintf(stderr, "iocp服务器创建失败\n");
		return;
	}

	mainloop();
}


//之后应该改成多线程
void iocpserver::mainloop() {
	while (1) {
		Sleep(1000);
		std::cout << "client:<" << client_count_ << "> msg:<"<<msg_count_<<">\n";
		msg_count_ = 0;
		chrono_time_.FlushTime();
	}
}








//打开网络库/启动网络库
int iocpserver::winsockinit() {
	int ret = -1;
	do {
		WORD version = MAKEWORD(2, 2);
		WSADATA wsadata;
		//成功返回0
		//所以
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





//创建 监听socket 并且关联 完成端口
int iocpserver::InitSocket() {
	int ret = 0;
	do {
		//注意必须是WSA_FLAG_OVERLAPPED   IOCP 模型 专用
		socket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (socket_ == INVALID_SOCKET) {
			fprintf(stderr, "创建 WSASocket 失败\n");
			ret = -1;
			break;
		}
		//将新创建的监听socket 与之前的完成端口 “关联” 起来
		if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket_), completion_port_, 0, 0)) {
		}
	} while (0);

	return ret;
}



//为监听socket 绑定
int iocpserver::bind(const char* ip, unsigned short prot) {
	SOCKADDR_IN addr;

	//addr.sin_addr.S_un.S_addr = inet_addr(ip);
	inet_pton(AF_INET, ip, &addr.sin_addr.s_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(prot);


	if (::bind(socket_, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		fprintf(stderr, "bind  失败\n");
		return -1;
	}
	return 0;
}


//传入的是监听队列的长度
int iocpserver::listen(size_t nlisten) {
	if (::listen(socket_, nlisten) == SOCKET_ERROR) {
		fprintf(stderr, "listen  失败\n");
		return -1;
	}
	return 0;
}




int iocpserver::Accept() {
	int ret = -1;
	do {
		/*
		这里不能用 WSAAccept
		因为需要在查询完成端口的时候判断出
		该消息的类型
		所以用扩展版
		*/

		//acceptEx 函数的指针
		//fprintf(stderr,"listen\n");
		LPFN_ACCEPTEX accept_func;
		GUID accept_guild = WSAID_ACCEPTEX;  //固定的guid



		DWORD bytes;

		//获取到了LPFN_ACCEPTEX 的函数指针--->即accept_func赋值
		ret = WSAIoctl(
			socket_, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&accept_guild, sizeof(accept_guild),
			&accept_func, sizeof(accept_func),
			&bytes, NULL, NULL
		);
		if (ret != 0) {
			ret = -1;
			fprintf(stderr, "获取accpet 函数地址 失败\n");
			break;
		}



		//创建一个用于链接的 socket
		//普通的accept是调用后会返回一个接受到的socket
		//但是异步不一样，需要先手动创建一个，然后绑定过去
		SOCKET accept_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (accept_socket == SOCKET_ERROR) {
			fprintf(stderr, "初始化accept socket  失败\n");
			ret = -1;
			break;
		}



		//设置其类型，到时候即可在完成端口处继续调用该函数去不停
		//的去要求获取新连接
		//so all 异步函数都是需要不停的去调用-----> 从队列中取出时，必须再次调用
		
		//理论上这种结构都应该被优化
		//使用连接池
		//因为大量的使用new ，会导致大量的内存碎片
		//所以应该启动连接池 -----------> asio 就启动了连接池
		std::unique_ptr<Connection> conn = std::make_unique<Connection>(accept_socket);
		

		
		//memset(accept_over.get(), 0, sizeof(accept_over));
		//accept_over->type = OverLapped::Accpet_type;


		DWORD n = 0;    //记录一下实际收到的长度
		//重点就是最后一个参数---->重叠结构的指针
		//其实就是自己设计的结构体
		//之后所有的运用（缓冲区）都应该由该结构体提供
		const int accept_results = accept_func(
			socket_,                  //监听socket
			accept_socket,            //连接socket--->就是新创建的socket
			conn->GetReadBuffer(),    //数据缓冲区
			0,                        //打算读取的数据大小    0--> 该accpet 仅仅只是为了接受连接，不进行数据的读写
			sizeof(sockaddr_in) + 16,   //本地地址的长度  
			sizeof(sockaddr_in) + 16,   //远程地址的长度
			&n,                         //实际收到的长度
			//注意这个做了个强转，因为参数类型是LPOVERLAPPED
			//所以只要OverLapped结构体的第一个成员变量是LPOVERLAPPED
			//即可进行强转！！！
			reinterpret_cast<LPOVERLAPPED>(conn->GetAcceptOverLapped())
		);

		if (!(accept_results == true || WSAGetLastError() == WSA_IO_PENDING)) {
			ret = -1;
			fprintf(stderr, "调用accept ex  失败\n");
			break;
		}



		//将accept_socket 关联到 完成端口
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(accept_socket), completion_port_, 0, 0);
		//清除智能指针的管理权限，该对象应该活下去
		conn.release();

	} while (0);
	return ret;
}




int iocpserver::init(const char* ip, unsigned short port, unsigned int nlisten) {
	int ret = 0;
	do {
		ret = winsockinit();   //打开网络库
		if (ret == -1) {
			fprintf(stderr, "初始化 winsock init 失败\n");
			break;
		}

		//创建完成端口
		completion_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (!completion_port_) {
			fprintf(stderr, "创建完成端口 失败\n");
			ret = -1;
			break;
		}

		//初始化监听socket
		if ((ret = InitSocket()) == -1) {
			break;
		}

		//绑定端口
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
	//更新写入缓冲区大小
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
		fprintf(stderr, "WSAsend 接受数据  失败\n");
	}
}



void iocpserver::AsyncRead(const Connection* conn) {
	auto con = const_cast<Connection*>(conn);
	auto over = con->GetReadOverLapped();

	over->wsa_buf.buf = static_cast<char*>(over->conn_->GetReadBuffer());
	over->wsa_buf.len = over->conn_->GetReadBufferSize();

	DWORD flags = 0;
	DWORD bytes_transferred = 0;  //传输的字节数

	const int recv_result = WSARecv(over->conn_->GetSocket(),
		&over->wsa_buf, 1, &bytes_transferred, &flags,
		reinterpret_cast<LPWSAOVERLAPPED>(over), NULL);

	if (!(recv_result == 0 || (recv_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))) {
		fprintf(stderr, "WSARecv 接受数据  失败\n");
	}
}