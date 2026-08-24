#pragma once
#include <unistd.h>
#include "core.hpp"
#include "HTTP.hpp"

#define BUFFER_INL(ret_type) template <usize bufferSize> ret_type inline HTTP::Buffer<bufferSize>::

namespace HTTP {

template <usize bufferSize>
class Buffer {
	static const usize minReadSize = 4;

public:
	u8 data[bufferSize - 24];	// Last cache line is reserved for unbounded memory loads
	u8 *readPtr, *scanPtr, *writePtr;

	ALWAYS_INLINE
	u8* get_end() {	// rename to mptr
		return data + sizeof(data);
	}

	ALWAYS_INLINE
	const u8* get_end() const {
		return data + sizeof(data);
	}

	usize compact() {
		const usize bytesUsed = (usize)(writePtr - readPtr);
		const usize scanOffset = (usize)(scanPtr - readPtr);

		MEMMOVE(data, readPtr, bytesUsed);
		readPtr = data;
		scanPtr = data + scanOffset;
		writePtr = data + bytesUsed;
		return (usize)(get_end() - writePtr);
	}

	isize read(i32 fd, usize bytes) {
		usize bytesFree = (usize)(get_end() - writePtr);

		if (bytesFree < bytes) {
			bytesFree = compact();
			if (bytesFree < minReadSize)
				return -2;
		}

		const usize bytesCapped = MIN(bytesFree, bytes);
		isize bytesRead = ::read(fd, writePtr, bytesCapped);
		if (bytesRead > 0)
			writePtr += (usize) bytesRead;
		return bytesRead;
	}

	isize write(i32 fd, usize bytes) {
		usize bytesCapped = MIN(bytes, (usize)(writePtr - readPtr));
		isize bytesWritten = ::write(fd, readPtr, bytesCapped);

		if (bytesWritten > 0)
			readPtr += bytesWritten;
		return bytesWritten;
	}

	void reset() {
		readPtr = data;
		writePtr = data;
		scanPtr = data;
	}

	bool is_full() {
		return writePtr >= get_end();
	}

	// Search
	usize find_line_end();
	usize find_header_end();
	isize match_field();
	isize match_mime();

	// String
	usize itoa10(usize number, char *bufferEnd);
	usize itoa16(usize number, char *bufferEnd);
	usize strtol10();
	usize strtol16();

	template <usize N>
	bool strcmp(const char (&string)[N]);

	template <usize N>
	bool strcasecmp(const char (&string)[N]);

	bool skip_spaces();

	// Adds
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

};
}

typedef HTTP::Buffer<HTTP_BUFFERSIZE> HTTP_Buffer;

#include "Buffer_add.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
