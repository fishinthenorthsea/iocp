#include<iostream>
#include<thread>
#include"TcpClient.hpp"


#define clientcount   10000
#define thread_count 4
TcpClient* client[clientcount];



struct msgg {
	char name[200];
	char passwd[200];
};

void sendthread(int id) {
	msgg msg[100];

	for (int i = 0; i < 100; i++) {
		strcpy(msg[i].name, "zwkzwkzwkzwk");
		strcpy(msg[i].passwd, "123456asdasdasdas");
	}

	//每个线程负责多个个客户端
	int nthread = clientcount / thread_count;

	int begin = (id - 1) * nthread;
	int end = id * nthread;
	int len = sizeof(msg);
	while (1) {
		for (int i = begin; i < end; i++) {
			client[i]->SendData(msg,len);
		}
	}
}



int main() {
//	client.run("127.0.0.1",9999);
	for (int i = 0; i < clientcount; i++) {
		client[i] = new TcpClient();
		client[i]->Connect("127.0.0.1", 9999);
	}

	for (int i = 0; i < thread_count; i++) {
		std::thread t(sendthread, i + 1);
		t.detach();
	}

	


	TcpClient my;
	my.run("127.0.0.1", 9999);

	getchar();
	return 0;
}