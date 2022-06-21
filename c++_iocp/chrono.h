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

	//����Ϊ��λ ����΢��Ϊ��λ * 1* 10��-6��
	double GetSecInterval() {
		return GetMicroSecInterval() * 0.000001;
	}

	//��ȡ��ǰ����һ�ε�ʱ���
	//��΢��Ϊ��λ
	long long GetMicroSecInterval() {
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - time_).count();
	}


	std::chrono::time_point<std::chrono::high_resolution_clock>time_;
};