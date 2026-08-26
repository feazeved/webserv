#pragma once
#include <ctime>

#include "core.hpp"

class Clock {
public:
	static std::time_t timeBegin, timeNow, timeElapsed;
	static std::tm *timeInfo;
	static isize secondsRef;

	static void init() {
		timeBegin = std::time(NULL);
	}

	static void update_time() {
		timeNow = std::time(NULL);
		timeElapsed = timeNow - timeBegin;
		timeInfo = std::localtime(&timeBegin);
	}

	// Does not call update time
	static u32 time_elapsed() {
		return (u32)timeElapsed;
	}

};
