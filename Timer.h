#pragma once

#include <chrono>
#include <string>
#include <cstdio>

class Timer
{
public:
	Timer(std::string inName) : name(inName)
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	~Timer() {
		Stop();
	}

	void Stop();

private:
	std::string name;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};