#pragma once
#include "Buffer.hpp"

CURSOR_INL
(bool) prepend(const u8 *ptr, usize length) {
	const usize readSize = (usize)(readPtr - memStart);
	if (readSize < length)
		return false;
	readPtr -= length;
	MEMCPY(readPtr, ptr, length);
	return true;
}

CURSOR_INL
(bool) insert(const u8 *ptr, usize length, usize insertIndex) {
	MEMCPY(memStart + insertIndex, ptr, length);
}

CURSOR_INL
(template <usize N> void) append(const char (&string)[N]) {
	MEMCPY_INLINE(writePtr, string, N - 1);
	writePtr += N - 1;
}

CURSOR_INL
(void) append(const u8 *ptr, usize length) {
	MEMCPY(writePtr, ptr, length);
	writePtr += length;
}

CURSOR_INL
(template <usize N> void) append_inline(const u8 *ptr, usize length) {
	MEMCPY_INLINE(writePtr, ptr, N);
	writePtr += length;
}

// Should be impossible for dst buffer to not fit
// TODO: Might remove MIN3 and have it overflow to guarantee behavior
CURSOR_INL
(usize) append(Cursor &src, usize length) {
	usize remainingSrc = src.writePtr - src.readPtr;	// How many bytes it has read
	usize remainingDst = memEnd - writePtr;	// How many bytes are free in the buffer
	usize appendLength = MIN3(length, remainingSrc, remainingDst);

	MEMCPY(writePtr, src.readPtr, appendLength);	// TODO: VERIFY CORRECTNESS
	src.readPtr += appendLength;
	writePtr += appendLength;
	return appendLength;
}

// TODO: No length checks
// TODO: separate functions
CURSOR_INL
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

// Copies the contents of another buffer into this buffer
// Good for defragmentation
CURSOR_INL
(void) copy(const Cursor& other) {
	const usize bytesUsed = (usize)(other.writePtr - other.readPtr);
	writePtr = memStart + bytesUsed;
	readPtr = memStart;
	MEMCPY(memStart, other.readPtr, bytesUsed);
}
