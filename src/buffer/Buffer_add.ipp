#pragma once
#include "Buffer.hpp"

// TODO: remove this function and just do it inside each function
static inline
char* s_original_ptr(u8* &ptr, usize delta) {
	char *optr = (char*) ptr;
	ptr += delta;
	return optr;
}

BUFFER_INL
(char*) prepend(const Span &span) {
	readPtr -= span.length;
	MEMCPY(readPtr, span.ptr, span.length);
	return (char*) readPtr;
}

BUFFER_INL
(char*) prepend(const char* ptr, usize length) {
	readPtr -= length;
	MEMCPY(readPtr, ptr, length);
	return (char*) readPtr;
}

BUFFER_INL_T
(usize N, char*) prepend(const char (&string)[N]) {
	readPtr -= (N - 1);
	MEMCPY_INLINE(readPtr, string, (N - 1));
	return (char*) readPtr;
}

BUFFER_INL_T
(usize N, char*) prepend_inline(const char* ptr, usize length) {
	readPtr -= length;
	MEMCPY_INLINE(readPtr, ptr, (N - 1));
	return (char*) readPtr;
}

BUFFER_INL
(char*) append(const char* ptr, usize length) {
	MEMCPY(writePtr, ptr, length);
	return s_original_ptr(writePtr, length);
}

BUFFER_INL
(char*) append(const Span &span) {
	MEMCPY(writePtr, span.ptr, span.length);
	return s_original_ptr(writePtr, span.length);
}

BUFFER_INL_T
(usize N, char*) append(const char (&string)[N]) {
	MEMCPY_INLINE(writePtr, string, N);	// Copies null terminator, but only 
	return s_original_ptr(writePtr, (N - 1));
}

BUFFER_INL_T
(usize N, char*) append_inline(const char* ptr, usize length) {
	MEMCPY_INLINE(writePtr, ptr, N);
	writePtr += length;
	return (char*) writePtr - length;
}

BUFFER_INL
(char*) append_digit10(usize number) {
	const usize maxLengthAligned = 24;
	char buffer[maxLengthAligned * 2];
	char *digitEnd = buffer + maxLengthAligned;
	usize digitLength = s_itoa10(number, digitEnd);

	char* digitStart = digitEnd - digitLength;
	MEMCPY_INLINE(writePtr, digitStart, maxLengthAligned);
	return s_original_ptr(writePtr, (isize)digitLength);
}

BUFFER_INL
(char*) append_digit16(usize number) {
	const usize maxLengthAligned = 16;
	char buffer[maxLengthAligned * 2];
	char *digitEnd = buffer + maxLengthAligned;
	usize digitLength = s_itoa16(number, digitEnd);

	char* digitStart = digitEnd - digitLength;
	MEMCPY_INLINE(writePtr, digitStart, maxLengthAligned);
	writePtr += digitLength;
	return (char*) writePtr - digitLength;
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
