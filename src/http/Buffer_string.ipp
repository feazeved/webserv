#pragma once
#include "Buffer.hpp"

BUFFER_INL
(usize) itoa10(usize number, char *bufferEnd) {
	*bufferEnd = 0;
	char *obuffer = --bufferEnd;
	do
	{
		*--bufferEnd = (char)((number % 10) + '0');
		number /= 10;
	}	while (number != 0);
	usize digitLength = (usize) (obuffer - bufferEnd);
	return digitLength;
}

BUFFER_INL
(usize) itoa16(usize number, char *bufferEnd) {
	static const char digits[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	*bufferEnd = 0;
	char *obuffer = --bufferEnd;
	do
	{
		*--bufferEnd = digits[(number % 16)];
		number /= 16;
	}	while (number != 0);
	usize digitLength = (usize) (obuffer - bufferEnd);
	return digitLength;
}

BUFFER_INL
(usize) strtol16() {
	usize value = 0;
	usize digit = 0;
	const u8 *ostr = linePtr;
	while (true) {
		digit = (usize) g_asciiLut[(u8)*linePtr];
		if (value >= (SIZE_MAX / 16 - 16))
			return SIZE_MAX;
		if (digit > 15)
			break;
		linePtr++;
		value = value * 16 + digit;
	}
	if (ostr == linePtr)
		return SIZE_MAX;
	return value;
}

BUFFER_INL
(usize) strtol10() {
	usize value = 0;
	usize digit = 0;
	const u8 *ostr = linePtr;
	while (true) {
		digit = (usize) g_asciiLut[(u8)*linePtr];
		if (value >= (SIZE_MAX / 10 - 10))
			return SIZE_MAX;
		if (digit > 9)
			break;
		linePtr++;
		value = value * 10 + digit;
	}
	if (ostr == linePtr)
		return SIZE_MAX;
	return value;
}

BUFFER_INL
(bool) skip_spaces() {
	while ((*linePtr == ' ' || *linePtr == '\t'))
		linePtr++;
	return MEMCMP(linePtr, "\r\n", 2) == 0;
}

// Compares and advances pointer if valid
BUFFER_INL
(template <usize N> bool) strcmp(const char (&string)[N]) {
	bool isMatch = MEMCMP(linePtr, string, N) != 0;
	linePtr += isMatch ? N : 0;
	return isMatch;
}

BUFFER_INL
(template <usize N> bool) strcasecmp(const char (&string)[N]) {
	u8 buffer[N];

	MEMCPY_INLINE(buffer, linePtr, N);
	for (usize i = 0; i < N; i++)
		buffer[i] |= 32;
	bool isMatch = MEMCMP(buffer, string, N) != 0;
	linePtr += isMatch ? N : 0;
	return isMatch;
}
