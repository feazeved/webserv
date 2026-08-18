#pragma once
#include "core.hpp"

static const u64 s_mask = 0x0FFFFFFFFFFFFFFF;

/*
	The result union will always have 8 bytes
	For payloads below 64 bits, there is no need for sentinels

	Sentinel values:
	- For 64 bit integers, MSB will be used as a sentinel
	- For pointers, MSB also but special return that copies MSB-1
	- For doubles, NAN
*/

template <typename Type>
union Result {

	u64 storage;
	Type result;

	bool ok() const {
		return !!(storage & ~s_mask);
	}

	Type value() const {
		Result rvalue;
		rvalue.storage = storage & s_mask;
		return rvalue.result;
	}

	// Sets the error value along with the sentinel bit
	void set_error(u64 value) {

	}
};
