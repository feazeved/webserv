#pragma once
#include "Buffer.hpp"

BUFFER_INL
(usize) s_itoa10(usize number, char *bufferEnd) {
	*bufferEnd = 0;
	char *const obuffer = bufferEnd;
	do {
		*--bufferEnd = (char)((number % 10) + '0');
		number /= 10;
	}	while (number != 0);
	return (usize)(obuffer - bufferEnd);
}

BUFFER_INL
(usize) s_itoa16(usize number, char *bufferEnd) {
	static const char digits[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	*bufferEnd = 0;
	char *const obuffer = bufferEnd;
	do {
		*--bufferEnd = digits[(number % 16)];
		number /= 16;
	}	while (number != 0);
	return (usize)(obuffer - bufferEnd);
}

BUFFER_INL
(usize) strtol16() {
	usize value = 0;
	usize digit = 0;
	const usize originalPos = readPos;
	while (true) {
		digit = (usize) g_asciiLut[data[readPos]];
		if (value >= (SIZE_MAX / 16 - 16))	// Constant comparison reduces branching
			return SIZE_MAX;
		if (digit > 15)
			break;
		readPos++;
		value = value * 16 + digit;
	}
	if (originalPos == readPos)
		return SIZE_MAX;
	return value;
}

BUFFER_INL
(usize) strtol10() {
	usize value = 0;
	usize digit = 0;
	const usize originalPos = readPos;
	while (true) {
		digit = (usize) data[readPos] - '0';
		if (value >= ((SIZE_MAX - 9) / 10))	// Constant comparison reduces branching
			return SIZE_MAX;
		if (digit > 9)
			break;
		readPos++;
		value = value * 10 + digit;
	}
	if (originalPos == readPos)
		return SIZE_MAX;
	return value;
}

BUFFER_INL
(bool) skip_spaces() {
	while ((data[readPos] == ' ' || data[readPos] == '\t'))
		readPos++;
	return MEMCMP(data + readPos, "\r\n", 2) != 0;
}

// Compares and advances pointer if valid
BUFFER_INL_T
(usize N, bool) strcmp(const char (&string)[N]) {
	const usize strLength = N - 1;
	bool isMatch = MEMCMP(data + readPos, string, strLength) == 0;
	readPos += isMatch ? strLength : 0;
	return isMatch;
}

BUFFER_INL_T
(usize N, bool) strcasecmp(const char (&string)[N]) {
	u8 buffer[N];
	const usize strLength = N - 1;

	MEMCPY_INLINE(buffer, data + readPos, strLength);
	for (usize i = 0; i < strLength; i++)
		buffer[i] |= 32;
	bool isMatch = MEMCMP(buffer, string, strLength) == 0;
	readPos += isMatch ? strLength : 0;
	return isMatch;
}
