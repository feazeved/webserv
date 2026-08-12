#pragma once
#include "Buffer.hpp"

CURSOR_INL
(isize) find_line_end() {
	const u8 *const searchEnd = writePtr - memStart >= 3 ? writePtr - 3 : memStart;
	lineEnd = NULL;

	while (scanPtr < searchEnd) {
		if (MEMCMP(scanPtr, "\r\n", 2) == 0) {
			lineEnd = scanPtr;
			scanPtr += 2;
			if (MEMCMP(scanPtr, "\r\n", 2) == 0) {
				scanPtr += 2;
				return 2; // Found header end
			}
			return 1; // Found line end
		}
		else
			scanPtr++;
	}
	return 0;
}

// Does not update start and end (not for line parsing)
CURSOR_INL
(bool) find_header_end() {
	const u8 *const searchEnd = writePtr - memStart >= 3 ? writePtr - 3 : memStart;

	while (scanPtr < searchEnd) {
		if (MEMCMP(scanPtr, "\r\n\r\n", 4) == 0) {
			scanPtr += 4;
			return true; // Found header end
		}
		else
			scanPtr++;
	}
	return false;
}

/* (IMPORTANT) This function presumes 32 byte padding
This function performs a 32 byte load of a field delimited by : then compares
against a table of reference strings to find a match. Because MEMCMP length is
fixed, the compiler automatically vectorizes the comparison

Returns: 0 on no matches, -1 on errors or
		index associated with the string compared

TODO:	Finding can be two operations, Setting or can be one operation
		Move table to init, Automate the creation of the enums from the table
*/
template <usize count, usize size>
static inline
isize s_match(const u8 *ptr, usize length, const u8 (&ltable)[count][size]) {
	u8 buffer[size * 2];

	MEMCPY_INLINE(buffer, ptr, size);
	for (usize i = 0; i < size; i++)
		buffer[i] |= 32;
	MEMSET_INLINE(buffer + length, 0, size);

	for (usize i = 0; i < count; i++) {
		if (MEMCMP(ltable[i], buffer, size) == 0)
			return (isize)i + 1;
	}
	return 0;
}

CURSOR_INL
(isize) match_field() {
	static const u8 ltable[][32] = {
		"status", "location", "transfer-encoding", "content-length", 
		"content-type", "host", "connection", "accept"};	// TODO: add sse, remove location

	u8 *optr = readPtr;

	while (readPtr < lineEnd && *readPtr != ':')
		readPtr++;
	usize length = (usize)(readPtr - optr);
	if (length >= sizeof(*ltable) || *readPtr != ':')
		return (*readPtr != ':') ? -1 : 0;
	readPtr++;
	return s_match(optr, length, ltable);
}

CURSOR_INL
(isize) match_mime() {
	static const u8 ltable[][8] = {
		"html", "htm", "css", "json", "js", 
		"png", "jpg", "jpeg", "gif", "txt"};

	const usize minLength = (usize) MAX(0, lineEnd - readPtr - 3);
	const u8 *searchLength = readPtr + MIN(minLength, 252);

	while (readPtr < searchLength && *readPtr != '.')
		readPtr++;
	if (readPtr >= searchLength)
		return -1;
	readPtr++;
	return s_match(readPtr, 5, ltable);
}
