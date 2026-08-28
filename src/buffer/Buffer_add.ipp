#pragma once
#include "Buffer.hpp"

// TODO: remove this function and just do it inside each function
static inline
char* s_original_ptr(u8 *data, usize &pos, usize delta) {
	char *optr = (char*) data + pos;
	pos += delta;
	return optr;
}

BUFFER_INL
(char*) prepend(const Span &span) {
	readPos -= span.length;
	MEMCPY(data + readPos, span.ptr, span.length);
	return (char*) data + readPos;
}

BUFFER_INL
(char*) prepend(const char* ptr, usize length) {
	readPos -= length;
	MEMCPY(data + readPos, ptr, length);
	return (char*) data + readPos;
}

BUFFER_INL_T
(usize N, char*) prepend(const char (&string)[N]) {
	readPos -= (N - 1);
	MEMCPY_INLINE(data + readPos, string, (N - 1));
	return (char*) data + readPos;
}

BUFFER_INL_T
(usize N, char*) prepend_inline(const char* ptr, usize length) {
	readPos -= length;
	MEMCPY_INLINE(data + readPos, ptr, (N - 1));
	return (char*) data + readPos;
}

BUFFER_INL
(char*) append(const char* ptr, usize length) {
	MEMCPY(data + writePos, ptr, length);
	return s_original_ptr(data, writePos, length);
}

BUFFER_INL
(char*) append(const Span &span) {
	MEMCPY(data + writePos, span.ptr, span.length);
	return s_original_ptr(data, writePos, span.length);
}

BUFFER_INL_T
(usize N, char*) append(const char (&string)[N]) {
	MEMCPY_INLINE(data + writePos, string, N);	// Copies null terminator, advances past content only
	return s_original_ptr(data, writePos, (N - 1));
}

BUFFER_INL_T
(usize N, char*) append_inline(const char* ptr, usize length) {
	MEMCPY_INLINE(data + writePos, ptr, N);
	return s_original_ptr(data, writePos, length);
}

BUFFER_INL
(char*) append_digit10(usize number) {
	const usize maxLengthAligned = 24;
	char buffer[maxLengthAligned * 2];
	char *digitEnd = buffer + maxLengthAligned;
	usize digitLength = s_itoa10(number, digitEnd);

	char* digitStart = digitEnd - digitLength;
	MEMCPY_INLINE(data + writePos, digitStart, maxLengthAligned);
	return s_original_ptr(data, writePos, digitLength);
}

BUFFER_INL
(char*) append_digit16(usize number) {
	const usize maxLengthAligned = 16;
	char buffer[maxLengthAligned * 2];
	char *digitEnd = buffer + maxLengthAligned;
	usize digitLength = s_itoa16(number, digitEnd);

	char* digitStart = digitEnd - digitLength;
	MEMCPY_INLINE(data + writePos, digitStart, maxLengthAligned);
	return s_original_ptr(data, writePos, digitLength);
}

// Should be impossible for dst buffer to not fit
// TODO: Might remove MIN3 and have it overflow to guarantee behavior
BUFFER_INL
(usize) append_buffer(Buffer &src, usize length) {
	usize remainingSrc = src.writePos - src.readPos;	// How many bytes remain unread
	usize remainingDst = sizeof(data) - writePos;	// How many bytes are free in the buffer
	usize appendLength = MIN3(length, remainingSrc, remainingDst);

	MEMCPY(data + writePos, src.data + src.readPos, appendLength);
	src.readPos += appendLength;
	src.scanPos = (src.scanPos >= src.readPos) ? src.scanPos : src.readPos;
	writePos += appendLength;
	return appendLength;
}
