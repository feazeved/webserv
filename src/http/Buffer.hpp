#pragma once

#include <unistd.h>

#include "core.hpp"

template <usize bufferSize>
class Buffer {
public:
	u8 data[bufferSize];
	u32 readOffset, writeOffset;

	isize read(i32 fd, usize bytes) {
		if (writeOffset + bytes > sizeof(data))
			return -1;	// ERROR: Line is too long
	
		isize bytesRead = ::read(fd, data + writeOffset, bytes);
		if (bytesRead <= 0)
			return bytesRead;	// TODO: Need to distinguish between first read fail and other read fails
		writeOffset += (usize) bytesRead;	// TODO: what do we do on failures?
	}

};