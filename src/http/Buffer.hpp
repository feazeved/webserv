#pragma once
#include <unistd.h>
#include "core.hpp"

#define BUFFER_INL(ret_type) template <usize bufferSize> ret_type inline Buffer<bufferSize>::

template <usize bufferSize>
class Buffer {
public:
	union {
		u8 rawData[bufferSize];
		struct {
			u8 data[bufferSize - sizeof(usize) * 8];	// Last cache line is reserved for unbounded memory loads
			usize reserved[4];
			u8 *readPtr, *writePtr, *linePtr, *lineEnd;
		};
	};

	isize read(i32 fd, usize bytes) {
		if (writePtr + bytes > data + sizeof(data))
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
			MEMMOVE(data, readPtr, 32);
			writePtr = data + tailBytes;
			readPtr = data;
		}
		return bytesWritten;
	}

	void reset() {
		readPtr = data;
		writePtr = data;
	}

	bool is_full() {
		return writePtr >= (data + sizeof(data));
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

	usize append(Buffer &src, usize length);
	void append_digit10(usize number);

	void copy(const Buffer& other);
	bool insert(const u8 *ptr, usize length, usize insertIndex);

Buffer()
	: readPtr(data), writePtr(data), linePtr(data), lineEnd(NULL)
	{
	}
};

#include "Buffer_add.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
