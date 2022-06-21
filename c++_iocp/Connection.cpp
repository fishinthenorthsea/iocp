#include"Connection.h"



OverLapped* CreadOverlapped(OverLapped::Type t) {
	OverLapped* over = new OverLapped;
	memset(over, 0, sizeof(OverLapped));
	over->type = t;
	return over;

}

Connection::Connection(const SOCKET socket)
:accept_socket_(socket)
, write_size(0)
, write_buffer_(nullptr)
, send_byte(0)
, send_all(0)
, read_cnt(0)
, connect_overlapped_(CreadOverlapped(OverLapped::Connect_type))
, accept_overlapped_(CreadOverlapped(OverLapped::Accept_type))
, read_overlapped_(CreadOverlapped(OverLapped::Read_type))
, write_overlapped_(CreadOverlapped(OverLapped::Write_type))
{
	connect_overlapped_->conn_ = this;
	accept_overlapped_->conn_ = this;
	read_overlapped_->conn_ = this;
	write_overlapped_->conn_ = this;
}