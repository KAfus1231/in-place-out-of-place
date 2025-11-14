#include "Timer.h"

Timer::~Timer()
{
	stop();
}

void Timer::start()
{
	bool expect = false;
	if (!running.compare_exchange_strong(expect, true)) return;
}

void Timer::stop()
{
}
