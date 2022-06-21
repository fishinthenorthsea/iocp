#include"myasio.h"




void myasio::run(const char* ip, unsigned short port) {
	server.run(ip,port);
}