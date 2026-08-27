#pragma once
#include <unistd.h>

#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"

#define BUFFER_INL(ret_type) \
	template <usize bufferSize> inline ret_type Buffer<bufferSize>::

#define BUFFER_INL_T(tmpl_param, ret_type) \
	template <usize bufferSize> template <tmpl_param> inline ret_type Buffer<bufferSize>::

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
	usize capacity() const {
		return sizeof(data);
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
	isize check_target(Span &path, Span &query);

	// String
	static usize s_itoa10(usize number, char *bufferEnd);
	static usize s_itoa16(usize number, char *bufferEnd);
	usize strtol10();
	usize strtol16();

	template <usize N> bool strcmp(const char (&string)[N]);
	template <usize N> bool strcasecmp(const char (&string)[N]);

	bool skip_spaces();

	// Appends and Prepends
	template <usize N> char* append(const char (&string)[N]);				// Implicit
	template <usize N> char* append_inline(const char* ptr, usize length);	// Explicit
	char* append(const char* ptr, usize length);
	char* append(const Span &span);	// TODO: Is it better to have const ref or normal

	template <usize N> char* prepend(const char (&string)[N]);
	template <usize N> char* prepend_inline(const char* ptr, usize length);
	char* prepend(const char* ptr, usize length);
	char* prepend(const Span &span);
	
	usize append_buffer(Buffer &src, usize length);
	char* append_mime(u8 mimeIndex);

	char* append_digit10(usize number);
	char* append_digit16(usize number);

	void bufcpy(const Buffer& other) {
		const usize bytesUsed = (usize)(other.writePtr - other.readPtr);
		writePtr = data + bytesUsed;
		readPtr = data;
		scanPtr = data;
		MEMCPY(data, other.readPtr, bytesUsed);
	}

	operator char*() {
		return (char*)readPtr;
	}

	u8& operator*() {
		return *writePtr;
	}
};

typedef Buffer<HTTP_BUFFERSIZE> HTTP_Buffer;
typedef Buffer<8 * 1024> Buffer8;
typedef Buffer<16 * 1024> Buffer16;
typedef Buffer<64 * 1024> Buffer64;

#include "Buffer_add.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
