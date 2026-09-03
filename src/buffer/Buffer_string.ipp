#pragma once
#include "Buffer.hpp"

// BUFFER_INL
// (usize) qstrtol16() {
// 	usize value = 0;
// 	usize digit = 0;
// 	const usize originalPos = readPos;
// 	while (true) {
// 		digit = (usize) g_asciiLut[data[readPos]];
// 		if (value >= (SIZE_MAX / 16 - 16))	// Constant comparison reduces branching
// 			return SIZE_MAX;
// 		if (digit > 15)
// 			break;
// 		readPos++;
// 		value = value * 16 + digit;
// 	}
// 	if (originalPos == readPos)
// 		return SIZE_MAX;
// 	return value;
// }

// BUFFER_INL
// (usize) qstrtol10() {
// 	usize value = 0;
// 	usize digit = 0;
// 	const usize originalPos = readPos;
// 	while (true) {
// 		digit = (usize) data[readPos] - '0';
// 		if (value >= ((SIZE_MAX - 9) / 10))	// Constant comparison reduces branching
// 			return SIZE_MAX;
// 		if (digit > 9)
// 			break;
// 		readPos++;
// 		value = value * 10 + digit;
// 	}
// 	if (originalPos == readPos)
// 		return SIZE_MAX;
// 	return value;
// }

BUFFER_INL
(bool) skip_spaces() {
	while ((data[readPos] == ' ' || data[readPos] == '\t'))
		readPos++;
	return MEMCMP(data + readPos, "\r\n", 2) != 0;
}

BUFFER_INL
(Span) get_field_value(usize readEnd) {
	Span result = {};
	while ((data[readPos] == ' ' || data[readPos] == '\t'))
		readPos++;
	if (readPos >= readEnd)
		return result;
	usize valueEnd = readEnd;
	while ((data[valueEnd - 1] == ' ' || data[valueEnd - 1] == '\t'))
		valueEnd--;
	readPos = scanPos;
	result.ptr = (char*)data + readPos;
	result.size = valueEnd - readPos;
	return result;
}

// Compares and advances pointer if valid
BUFFER_INL_T
(usize N, bool) strcmp(const char (&string)[N]) {
	const usize strLength = N - 1;
	bool isMatch = MEMCMP(data + readPos, string, strLength) == 0;
	readPos += isMatch ? strLength : 0;
	return isMatch;
}

// BUFFER_INL_T
// (usize N, bool) strcasecmp(const char (&string)[N]) {
// 	u8 buffer[N];
// 	const usize strLength = N - 1;

// 	MEMCPY_INLINE(buffer, data + readPos, strLength);
// 	for (usize i = 0; i < strLength; i++)
// 		buffer[i] |= 32;
// 	bool isMatch = MEMCMP(buffer, string, strLength) == 0;
// 	readPos += isMatch ? strLength : 0;
// 	return isMatch;
// }
