#pragma once
#include "includes.h"

class Timer
{
public:
	Timer() = default;
	
	// гюопер йнохпнбюмхъ онрнйнб
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	~Timer();

	void start();
	void stop();

private:
	std::atomic<bool> running{ false };
	std::atomic<int> seconds{ 0 };
};