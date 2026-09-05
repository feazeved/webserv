#pragma once
#include <unistd.h>

#include "core.hpp"
#include "tables.hpp"
#include "webserv.hpp"
#include "Span.hpp"
#include "pure_functions.hpp"

#define BUFFER_INL(ret_type) \
	template <usize bufferSize> inline ret_type Buffer<bufferSize>::

#define BUFFER_INL_T(tmpl_param, ret_type) \
	template <usize bufferSize> template <tmpl_param> inline ret_type Buffer<bufferSize>::

template <usize bufferSize>
struct Buffer {
	static const usize minReadSize = 4;
	u64 clobberPre;
	u8 data[bufferSize - 16 - (3 * sizeof(usize))];	// Trailing storage pads unbounded memory loads
	u64 clobberPost;
	usize writePos, readPos, scanPos;

	inl u8* get_end() {	// rename to mptr
		return data + sizeof(data);
	}

	inl Span get_span() {
		Span result = {(char*)data + readPos, writePos - readPos};
		return result;
	}

	inl const u8* get_end() const {
		return data + sizeof(data);
	}

	inl usize size() const {
		return writePos - readPos;
	}

	inl usize capacity() const {
		return sizeof(data);
	}

	inl usize bytes_free() const {
		return sizeof(data) - writePos;
	}

	inl usize reserve(usize bytes) {
		usize bytesFree = bytes_free();
		if (bytes >= bytesFree)
			bytesFree = compact();
		return bytesFree;
	}

	inl bool is_full() {
		return writePos >= sizeof(data);
	}

	void clear() {
		readPos = 0;
		writePos = 0;
		scanPos = 0;
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

	// HTTP
	isize dechunk(Buffer& tmp, usize &chunkSize, usize &bodySize);
	isize decode(int writeFd, usize &chunkSize, usize &bodySize);

	// Search
	Span find_line_end();
	Span find_header_end();
	Span find_char(u8 c);

	template <usize N> bool strcmp(const char (&string)[N]);
	// template <usize N> bool strcasecmp(const char (&string)[N]);

	bool skip_spaces();
	Span get_field_value(usize readEnd);

	// Appends and Prepends
	char* append_char(char c);
	template <usize N> char* append(const char (&string)[N]);				// Implicit
	template <usize N> char* append_inline(const char* ptr, usize length);	// Explicit
	char* append(const char* ptr, usize length);
	char* append(const Span &span);	// TODO: Is it better to have const ref or normal

	template <usize N> char* prepend(const char (&string)[N]);
	template <usize N> char* prepend_inline(const char* ptr, usize length);
	char* prepend(const char* ptr, usize length);
	char* prepend(const Span &span);

	// Append Special
	usize append_buffer(Buffer &src, usize length);
	char* append_mime(u8 mimeIndex);

	char* append_digit10(usize number);
	char* append_digit16(usize number);
	char* append_url_component(const char *ptr, usize length);
	char* append_html(char *ptr, usize length);

	char* memset(u8 byte, usize length);
	template <usize N> char* memset_inline(u8 byte, usize length);

	void bufcpy(const Buffer& other) {
		const usize bytesUsed = other.writePos - other.readPos;
		writePos = bytesUsed;
		readPos = 0;
		scanPos = 0;
		MEMCPY(data, other.data + other.readPos, bytesUsed);
	}

	inl operator char*() { return (char*)(data + readPos); }
	inl char* rptr() { return (char*)(data + readPos); }
	inl u8& rptr(usize index) { return *(data + readPos + index); }
	inl char* wptr() { return (char*)(data + writePos); }
	inl u8& wptr(usize index) { return *(data + writePos + index); }
	inl char* sptr() { return (char*)(data + scanPos); }
	inl u8& sptr(usize index) { return *(data + scanPos + index); }
	inl u8& operator*() { return data[writePos]; }
};

typedef Buffer<8 * 1024> Buffer8;
typedef Buffer<16 * 1024> Buffer16;
typedef Buffer<32 * 1024> Buffer32;
typedef Buffer<64 * 1024> Buffer64;

#include "Buffer_add.ipp"
#include "Buffer_add_special.ipp"
#include "Buffer_search.ipp"
#include "Buffer_string.ipp"
#include "Buffer_http.ipp"
