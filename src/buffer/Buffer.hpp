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
	u8 data[bufferSize - (3 * sizeof(usize))];	// Trailing storage pads unbounded memory loads
	usize readPos, scanPos, writePos;

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
		return writePos - readPos;
	}

	ALWAYS_INLINE
	usize capacity() const {
		return sizeof(data);
	}

	ALWAYS_INLINE
	usize bytes_free() const {
		return sizeof(data) - writePos;
	}

	void clear() {
		readPos = 0;
		writePos = 0;
		scanPos = 0;
	}

	bool is_full() {
		return writePos >= sizeof(data);
	}

	usize compact() {
		const usize bytesUsed = writePos - readPos;
		const usize scanOffset = scanPos - readPos;

		MEMMOVE(data, data + readPos, bytesUsed);
		readPos = 0;
		scanPos = scanOffset;
		writePos = bytesUsed;
		return sizeof(data) - writePos;
	}

	isize read(int fd, usize bytes) {
		usize bytesFree = sizeof(data) - writePos;

		if (bytesFree < bytes) {
			bytesFree = compact();
			if (bytesFree < minReadSize)
				return -2;
		}

		const usize bytesCapped = MIN(bytesFree, bytes);
		isize bytesRead = ::read(fd, data + writePos, bytesCapped);
		if (bytesRead > 0)
			writePos += (usize) bytesRead;
		return bytesRead;
	}

	isize write(int fd, usize bytes) {
		usize bytesCapped = MIN(bytes, writePos - readPos);
		isize bytesWritten = ::write(fd, data + readPos, bytesCapped);

		if (bytesWritten > 0) {
			readPos += (usize) bytesWritten;
			scanPos = (scanPos >= readPos) ? scanPos : readPos;
		}
		return bytesWritten;
	}

	char* memset(u8 byte, usize length) {
		MEMSET(data + writePos, byte, length);
		return s_original_ptr(data, writePos, length);
	}

	template <usize N>
	char* memset_inline(u8 byte, usize length) {
		MEMSET_INLINE(data + writePos, byte, N);
		return s_original_ptr(data, writePos, length);
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
		const usize bytesUsed = other.writePos - other.readPos;
		writePos = bytesUsed;
		readPos = 0;
		scanPos = 0;
		MEMCPY(data, other.data + other.readPos, bytesUsed);
	}

	operator char*() { return (char*)(data + readPos); }
	char* rptr() { return (char*)(data + readPos); }
	char* wptr() { return (char*)(data + writePos); }
	char* sptr() { return (char*)(data + scanPos); }

	u8& operator*() {
		return data[writePos];
	}
};

typedef Buffer<HTTP_BUFFERSIZE> HTTP_Buffer;
typedef Buffer<8 * 1024> Buffer8;
typedef Buffer<16 * 1024> Buffer16;
typedef Buffer<64 * 1024> Buffer64;

#include "Buffer_add.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
#include "Buffer_http.ipp"
