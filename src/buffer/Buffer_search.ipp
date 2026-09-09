#pragma once
#include "Buffer.hpp"

// TODO: Create a 2 byte linear search

BUFFER_INL
(Span) find_line_end() {
	u8 tmp[2];
	MEMCPY_INLINE(tmp, data + writePos, 2);
	MEMCPY_INLINE(data + writePos, "\r\n", 2);
	while (MEMCMP(data + scanPos, "\r\n", 2) != 0)
		scanPos++;
	MEMCPY_INLINE(data + writePos, tmp, 2);
	if (scanPos < writePos) {
		Span result = {(char*)data + readPos, scanPos - readPos};
		scanPos += 2;
		return result;
	}
	scanPos -= scanPos > readPos;
	Span result = {NULL, SIZE_MAX};
	return result;
}

BUFFER_INL
(Span) find_header_end() {
	u8 tmp[4];
	MEMCPY_INLINE(tmp, data + writePos, 4);
	MEMCPY_INLINE(data + writePos, "\r\n\r\n", 4);
	while (MEMCMP(data + scanPos, "\r\n\r\n", 4) != 0)
		scanPos++;
	MEMCPY_INLINE(data + writePos, tmp, 4);
	if (scanPos + 4 <= writePos) {
		Span result = {(char*)data + readPos, scanPos - readPos};
		scanPos += 4;
		return result;
	}
	scanPos -= MIN(scanPos - readPos, (usize)3);
	Span result = {NULL, SIZE_MAX};
	return result;
}

// Searches for \n without \r prefix
BUFFER_INL
(Span) find_cgi_line_end() {
	u8 tmp = data[writePos];
	data[writePos] = '\n';
	while (data[scanPos] != '\n')
		scanPos++;
	data[writePos] = tmp;
	if (scanPos < writePos) {
		usize lineEnd = scanPos;
		if (lineEnd > readPos && data[lineEnd - 1] == '\r')
			lineEnd--;
		Span result = {(char*)data + readPos, lineEnd - readPos};
		scanPos++;
		return result;
	}
	Span result = {NULL, SIZE_MAX};
	return result;
}

// Searches for \r\n\r\n, \n\r\n and \n\n
BUFFER_INL
(Span) find_cgi_header_end() {
	u8 tmp[3];
	MEMCPY_INLINE(tmp, data + writePos, 3);
	MEMCPY_INLINE(data + writePos, "\n\n\n", 3);
	while (data[scanPos] != '\n' || (data[scanPos + 1] != '\n' && MEMCMP(data + scanPos + 1, "\r\n", 2) != 0))
		scanPos++;
	const usize nextPos = scanPos + (data[scanPos + 1] == '\n' ? 2 : 3);
	MEMCPY_INLINE(data + writePos, tmp, 3);
	if (nextPos <= writePos) {
		const usize headerEnd = scanPos - (scanPos > readPos && data[scanPos - 1] == '\r');
		Span result = {(char*)data + readPos, headerEnd - readPos};
		scanPos = nextPos;
		return result;
	}
	scanPos -= MIN(scanPos - readPos, (usize)2);
	Span result = {NULL, SIZE_MAX};
	return result;
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

BUFFER_INL
(bool) skip_spaces() {
	while ((data[readPos] == ' ' || data[readPos] == '\t'))
		readPos++;
	return MEMCMP(data + readPos, "\r\n", 2) != 0;
}
