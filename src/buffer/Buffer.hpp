#pragma once
#include <unistd.h>
#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"

#define BUFFER_INL(ret_type) template <usize bufferSize> ret_type inline Buffer<bufferSize>::

template <usize bufferSize>
class Buffer {
public:
	typedef Buffer<HTTP_BUFFERSIZE> HTTP_Buffer;
	static const usize minReadSize = 4;
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

	ALWAYS_INLINE
	usize size() const {
		return writePtr - readPtr;
	}

	ALWAYS_INLINE
	usize bytes_free() const {
		return data + sizeof(data) - writePtr;
	}

	void clear() {
		readPtr = data;
		writePtr = data;
		scanPtr = data;
	}

	bool is_full() {
		return writePtr >= get_end();
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

	isize read(int fd, usize bytes) {
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

	isize write(int fd, usize bytes) {
		usize bytesCapped = MIN(bytes, (usize)(writePtr - readPtr));
		isize bytesWritten = ::write(fd, readPtr, bytesCapped);

		if (bytesWritten > 0)
			readPtr += bytesWritten;
		return bytesWritten;
	}

	// HTTP
	isize dechunk(HTTP_Buffer& tmp, usize &chunkSize, usize &bodySize);
	isize decode(int writeFd, usize &chunkSize, usize &bodySize);

	// Search
	usize find_line_end();
	usize find_header_end();
	isize match_field();
	isize match_mime();
	isize check_target(Span16 &path, Span16 &query);

	// String
	usize itoa10(usize number, char *bufferEnd);
	usize itoa16(usize number, char *bufferEnd);
	usize strtol10();
	usize strtol16();

	template <usize N> bool strcmp(const char (&string)[N]);
	template <usize N> bool strcasecmp(const char (&string)[N]);

	bool skip_spaces();

	// Appends and Prepends
	template <usize N> void append(const char (&string)[N]);				// Implicit
	template <usize N> void append_inline(const char* ptr, usize length);	// Explicit
	void append(const char* ptr, usize length);

	template <usize N> void prepend(const char (&string)[N]);
	template <usize N> void prepend_inline(const char* ptr, usize length);
	void prepend(const char* ptr, usize length);
	
	usize append_buffer(Buffer &src, usize length);
	void append_mime(u8 mimeIndex);
	void append_digit10(usize number);

	Buffer& operator=(const Buffer& other) {
		const usize bytesUsed = (usize)(other.writePtr - other.readPtr);
		writePtr = data + bytesUsed;
		readPtr = data;
		scanPtr = data;
		MEMCPY(data, other.readPtr, bytesUsed);
	}
};
typedef Buffer<HTTP_BUFFERSIZE> HTTP_Buffer;

#include "Buffer_add.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
