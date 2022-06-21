#pragma once

enum MessageTypes {
	T_Login = 0,
	T_Login_Result,
	T_Logout,
	T_Logout_Result,
	T_Error
};


struct MesageHeader {
	size_t len_;
	size_t type_;
};




class Login :public MesageHeader {
public:
	Login() {
		len_ = sizeof(Login);
		type_ = T_Login;
	}
	char name_[32];
	char passwd_[32];
};






class Login_Result :public MesageHeader {
public:
	Login_Result() {
		len_ = sizeof(Login_Result);
		type_ = T_Login_Result;
	}
	int result_;
};




class Logout :public MesageHeader {
public:
	Logout() {
		len_ = sizeof(Logout);
		type_ = T_Logout;
	}
	char name_[32];
	char passwd_[32];
};



class Logout_Result :public MesageHeader {
public:
	Logout_Result() {
		len_ = sizeof(Logout_Result);
		type_ = T_Logout_Result;
	}
	int result_;
};
