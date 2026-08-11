#pragma once
#include <unistd.h>
#include "core.hpp"

#define CURSOR_INL(ret_type) ret_type inline Cursor::

struct Cursor {
	usize reserved[2];
	u8 *const memStart;
	u8 *const memEnd;
	u8 *readPtr, *writePtr, *linePtr, *lineEnd;

	isize read(i32 fd, usize bytes) {
		if (writePtr + bytes > memEnd)
			return -1;	// ERROR: Buffer overflow

		isize bytesRead = ::read(fd, writePtr, bytes);
		if (bytesRead < 0)
			return -2;
		writePtr += (usize) bytesRead;	// TODO: what do we do on failures?
		return bytesRead;
	}

	isize write(i32 fd, usize bytes) {
		usize bytesCapped = MIN(bytes, writePtr - readPtr);
		isize bytesWritten = ::write(fd, readPtr, bytesCapped);

		if (bytesWritten < 0)
			return bytesWritten;
		readPtr += bytesWritten;
		isize tailBytes = writePtr - readPtr;
		if (tailBytes <= 32) {	// Check if this is needed
			MEMMOVE(memStart, readPtr, 32);
			writePtr = memStart + tailBytes;
			readPtr = memStart;
		}
		return bytesWritten;
	}

	void reset() {
		readPtr = memStart;
		writePtr = memStart;
	}

	bool is_full() {
		return writePtr >= (memEnd);
	}

	isize find_line_end();
	bool find_header_end();
	isize match_field();
	isize match_mime();

	usize itoa10(usize number, char *bufferEnd);
	usize itoa16(usize number, char *bufferEnd);
	usize strtol10();
	usize strtol16();

	template <usize N>
	bool strcmp(const char (&string)[N]);

	template <usize N>
	bool strcasecmp(const char (&string)[N]);
	bool skip_spaces();

	template <usize N>
	void append(const char (&string)[N]);

	void append(const u8 *ptr, usize length);
	bool prepend(const u8 *ptr, usize length);

	template <usize N>
	void append_inline(const u8 *ptr, usize length);

	usize append(Cursor &src, usize length);
	void append_digit10(usize number);

	void copy(const Cursor& other);
	bool insert(const u8 *ptr, usize length, usize insertIndex);
};

template <usize bufferSize>
class Buffer {
public:
	u8 data[bufferSize - sizeof(Cursor)];	// Last cache line is reserved for unbounded memory loads
	Cursor cursor;

// Buffer()
// 	: readPtr(data), writePtr(data), linePtr(data), lineEnd(NULL)
// 	{
// 	}
};

#include "Buffer_add.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
