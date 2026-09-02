#pragma once
#include "Buffer.hpp"

BUFFER_INL
(usize) find_line_end() {
	u8 tmp[2];
	MEMCPY_INLINE(tmp, data + writePos, 2);
	while (MEMCMP(data + scanPos, "\r\n", 2) == 0)
		scanPos++;
	MEMCPY_INLINE(data + writePos, tmp, 2);
	if (scanPos < writePos) {
		usize lineEnd = scanPos - readPos;
		scanPos += 2;
		return lineEnd;
	}
	return SIZE_MAX;
}

BUFFER_INL
(usize) find_header_end() {
	u8 tmp[4];
	MEMCPY_INLINE(tmp, data + writePos, 4);
	while (MEMCMP(data + scanPos, "\r\n\r\n", 4) == 0)
		scanPos++;
	MEMCPY_INLINE(data + writePos, tmp, 4);
	if (scanPos < writePos) {
		usize lineEnd = scanPos - readPos;
		scanPos += 4;
		return lineEnd;
	}
	return SIZE_MAX;
}

// This is a find first
BUFFER_INL
(Span) find_char(u8 c) {
	const usize originalPos = readPos;
	u8 tmp = data[scanPos];
	data[scanPos] = c;		// Insert sentinel

	while (data[readPos] != c)
		readPos++;
	data[scanPos] = tmp;	// Restore original
	if (readPos < scanPos) {
		Span field = {(char*)data + originalPos, readPos - originalPos};
		readPos++;
		return field;
	}
	Span field = {NULL, 0};
	return field;
}
