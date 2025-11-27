#include "Timer.h"

Timer::~Timer()
{
	stop();
}

void Timer::start()
{
	bool expect = false;
	if (!running.compare_exchange_strong(expect, true)) return;

	worker = std::thread([this]()
		{
			while (running.load())
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (!running.load()) break;
				seconds.fetch_add(1);
			}
		});
}

void Timer::stop()
{
	bool expect = true;
	if (!running.compare_exchange_strong(expect, false)) return;

	if (worker.joinable())
		worker.join();
}

int Timer::elepsed() const
{
	return seconds.load();
}

int Timer::isRunning() const
{
	return running.load();
}

void Timer::reset()
{
	seconds.store(0);
}
