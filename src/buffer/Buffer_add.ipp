#pragma once
#include "Buffer.hpp"

BUFFER_INL
(void) prepend(const char* ptr, usize length) {
	readPtr -= length;
	MEMCPY(readPtr, ptr, length);
}

BUFFER_INL
(template <usize N> void) prepend(const char (&string)[N]) {
	readPtr -= N;
	MEMCPY(readPtr, string, N);
}

BUFFER_INL
(template <usize N> void) prepend_inline(const char* ptr, usize length) {
	readPtr -= length;
	MEMCPY_INLINE(writePtr, ptr, N);
}

BUFFER_INL
(void) append(const char* ptr, usize length) {
	MEMCPY(writePtr, ptr, length);
	writePtr += length;
}

BUFFER_INL
(template <usize N> void) append(const char (&string)[N]) {
	MEMCPY_INLINE(writePtr, string, N - 1);
	writePtr += N - 1;
}

BUFFER_INL
(template <usize N> void) append_inline(const char* ptr, usize length) {
	MEMCPY_INLINE(writePtr, ptr, N);
	writePtr += length;
}

BUFFER_INL
(void) append_mime(u8 mimeIndex) {
	static const u8 mimeStrings[][32] = MIME_STRINGS;

	const u8 *str = mimeStrings[mimeIndex];
	MEMCPY_INLINE(writePtr, str + 1, 24);
	writePtr += *str;
}

// Should be impossible for dst buffer to not fit
// TODO: Might remove MIN3 and have it overflow to guarantee behavior
BUFFER_INL
(usize) append_buffer(Buffer &src, usize length) {
	usize remainingSrc = src.writePtr - src.readPtr;	// How many bytes it has read
	usize remainingDst = get_end() - writePtr;	// How many bytes are free in the buffer
	usize appendLength = MIN3(length, remainingSrc, remainingDst);

	MEMCPY(writePtr, src.readPtr, appendLength);	// TODO: VERIFY CORRECTNESS
	src.readPtr += appendLength;
	writePtr += appendLength;
	return appendLength;
}

// TODO: No length checks
// TODO: separate functions
BUFFER_INL
(void) append_digit10(usize number) {
	char buffer[48];
	char *mid = buffer + 24;
	usize digitLength = itoa10(number, mid);
	char *digitStart = buffer + 24 - digitLength;

	*mid++ = '\r';
	*mid = '\n';

	MEMCPY_INLINE(writePtr, digitStart, 24);
	writePtr += digitLength + 2;
}
