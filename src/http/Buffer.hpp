#pragma once
#include <unistd.h>
#include "core.hpp"

#define CURSOR_INL(ret_type) ret_type inline Cursor::

// Start, readPtr, scanPtr, writePtr
struct Cursor {
	static const usize bytesPadding = (sizeof(u8*) * 8) / 2;
	static const usize minReadSize = 4;

	u8 *reserved[2];		// TODO: Need to figure out how to split the metadata so that:
	u8 *const memStart;		// [pad32 data pad32] instead of [data pad64]
	u8 *const memEnd;
	u8 *readPtr, *scanPtr, *writePtr, *lineEnd;

	usize compact() {
		const usize bytesUsed = (usize)(writePtr - readPtr);

		MEMMOVE(memStart, readPtr, bytesUsed);
		readPtr = memStart;
		writePtr = memStart + bytesUsed;
		return (usize)(memEnd - writePtr);
	}

	isize read(i32 fd, usize bytes) {
		usize bytesFree = (usize)(memEnd - writePtr);

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
		readPtr = memStart;
		writePtr = memStart;
		scanPtr = memStart;
		lineEnd = NULL;
	}

	bool is_full() {
		return writePtr >= memEnd;
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

	usize append(Cursor &src, usize length);
	void append_digit10(usize number);

	void copy(const Cursor& other);
	bool insert(const u8 *ptr, usize length, usize insertIndex);

	Cursor(u8* start, usize totalSize) :
		reserved(), memStart(start), memEnd(start + totalSize - sizeof(Cursor)),
		readPtr(start), writePtr(start) {
		}
};

template <usize bufferSize>
class Buffer {
public:
	u8 data[bufferSize - sizeof(Cursor)];	// Last cache line is reserved for unbounded memory loads
	Cursor cursor;

	// Constructors
	Buffer() : cursor (data, bufferSize) {}
};

#include "Buffer_add.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
