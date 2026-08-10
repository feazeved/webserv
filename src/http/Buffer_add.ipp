#pragma once
#include "Buffer.hpp"

BUFFER_INL
(bool) prepend(const u8 *ptr, usize length) {
	if (readPtr - data < length)
		return false;
	readPtr -= length;
	MEMCPY(readPtr, ptr, length);
}

BUFFER_INL
(bool) insert(const u8 *ptr, usize length, usize insertIndex) {
	MEMCPY(data + insertIndex, ptr, length);
}

BUFFER_INL
(template <usize N> void) append(const char (&string)[N]) {
	MEMCPY_INLINE(writePtr, string, N - 1);
	writePtr += N - 1;
}

BUFFER_INL
(void) append(const u8 *ptr, usize length) {
	MEMCPY(writePtr, ptr, length);
	writePtr += length;
}

BUFFER_INL
(template <usize N> void) append_inline(const u8 *ptr, usize length) {
	MEMCPY_INLINE(writePtr, ptr, N);
	writePtr += length;
}

// Should be impossible for dst buffer to not fit
// TODO: Might remove MIN3 and have it overflow to guarantee behavior
BUFFER_INL
(usize) append(Buffer &src, usize length) {
	usize remainingSrc = src.writePtr - src.readPtr;	// How many bytes it has read
	usize remainingDst = data + sizeof(data) - writePtr;	// How many bytes are free in the buffer
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
	writePtr += digitLength;
}

BUFFER_INL
(void) copy(const Buffer& other) {
	writePtr = other.writePtr - other.readPtr;
	readPtr = 0;
	MEMCPY(data, other.readPtr, writePtr);
}
