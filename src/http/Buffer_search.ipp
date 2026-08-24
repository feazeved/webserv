#pragma once
#include "Buffer.hpp"
#include "HTTP.hpp"

BUFFER_INL
(usize) find_line_end() {
	const u8 *const searchEnd = writePtr - data >= 2 ? writePtr - 1 : data;

	while (scanPtr < searchEnd) {
		if (MEMCMP(scanPtr, "\r\n", 2) == 0) {
			usize lineEnd = (usize)(scanPtr - readPtr);
			scanPtr += 2;
			return lineEnd; // Found line end
		}
		else
			scanPtr++;
	}
	return SIZE_MAX;
}

BUFFER_INL
(usize) find_header_end() {
	const u8 *const searchEnd = writePtr - data >= 4 ? writePtr - 3 : data;

	while (scanPtr < searchEnd) {
		if (MEMCMP(scanPtr, "\r\n\r\n", 4) == 0) {
			usize lineLength = (usize)(scanPtr - readPtr);
			scanPtr += 4;
			return lineLength; // Found header end
		}
		else
			scanPtr++;
	}
	return SIZE_MAX;
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

BUFFER_INL
(isize) match_field() {
	static const u8 ltable[][32] = FIELD_TABLE;

	u8 *optr = readPtr;

	while (readPtr < scanPtr && *readPtr != ':')
		readPtr++;
	usize length = (usize)(readPtr - optr);
	if (length >= sizeof(*ltable) || *readPtr != ':')
		return (*readPtr != ':') ? -1 : 0;
	readPtr++;
	return s_match(optr, length, ltable);
}

BUFFER_INL
(isize) match_mime() {
	static const u8 ltable[][8] = MIME_TABLE;

	const usize minLength = (usize) MAX(0, scanPtr - readPtr - 3);
	const u8 *searchLength = readPtr + MIN(minLength, 252);

	while (readPtr < searchLength && *readPtr != '.')
		readPtr++;
	if (readPtr >= searchLength)
		return -1;
	readPtr++;
	return s_match(readPtr, 5, ltable);
}

