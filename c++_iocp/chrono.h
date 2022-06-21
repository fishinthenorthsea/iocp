#pragma once
#include<chrono>



class chronoTimer {
public:
	chronoTimer() {
		FlushTime();
	}


	void FlushTime() {
		time_ = std::chrono::high_resolution_clock::now();
	}

	//以秒为单位 就是微单为单位 * 1* 10（-6）
	double GetSecInterval() {
		return GetMicroSecInterval() * 0.000001;
	}

	//获取当前和上一次的时间差
	//以微妙为单位
	long long GetMicroSecInterval() {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - time_).count();
	}


	std::chrono::time_point<std::chrono::high_resolution_clock>time_;
};