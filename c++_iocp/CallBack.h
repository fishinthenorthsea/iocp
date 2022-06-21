#pragma once
#include <functional>.
#include"Connection.h"

namespace CallBack {
	typedef std::function<void(const Connection*)> Onwrite;
	typedef std::function<void(const Connection*, void*, std::size_t)> Onread;

	typedef std::function<void(const Connection*)> Onconnected;
	typedef std::function<void(const Connection*)> Ondisconnected;
}



class CallExecutor {
public:
	void setonconnectd(CallBack::Onconnected on) { onconnectd_ = on; }
	void setondisconnectd(CallBack::Ondisconnected  on) { ondisconnectd_ = on; }
	void setonread(CallBack::Onread  on) { onread_ = on; }
	void setonwrite(CallBack::Onwrite  on) { onwrite_ = on; }





	CallBack::Onconnected onconnectd_;
	CallBack::Ondisconnected ondisconnectd_;
	CallBack::Onread onread_;
	CallBack::Onwrite onwrite_;
};