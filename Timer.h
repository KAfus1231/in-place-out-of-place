#pragma once
#include "includes.h"

class Timer
{
public:
	Timer() = default;
	
	// ÇÀÏĞÅÒ ÊÎÏÈĞÎÂÀÍÈß ÏÎÒÎÊÎÂ
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	~Timer();

	void start();
	void stop();

	int elepsed() const;
	int isRunning() const;
	void reset();

private:
	std::atomic<bool> running{ false };
	std::atomic<int> seconds{ 0 };
	std::thread worker;
};