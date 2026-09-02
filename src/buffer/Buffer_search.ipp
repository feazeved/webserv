#pragma once
#include "Buffer.hpp"

BUFFER_INL
(usize) find_line_end() {
	const usize searchEnd = writePos >= 2 ? writePos - 1 : 0;

	while (scanPos < searchEnd) {
		if (MEMCMP(data + scanPos, "\r\n", 2) == 0) {
			usize lineEnd = scanPos - readPos;
			scanPos += 2;
			return lineEnd; // Found line end
		}
		else
			scanPos++;
	}
	return SIZE_MAX;
}

BUFFER_INL
(usize) find_header_end() {
	const usize searchEnd = writePos >= 4 ? writePos - 3 : 0;

	while (scanPos < searchEnd) {
		if (MEMCMP(data + scanPos, "\r\n\r\n", 4) == 0) {
			usize lineLength = scanPos - readPos;
			scanPos += 4;
			return lineLength; // Found header end
		}
		else
			scanPos++;
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

	length = length >= size ? 0 : length;
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

	const usize originalPos = readPos;

	while (readPos < scanPos && data[readPos] != ':')
		readPos++;
	usize length = readPos - originalPos;
	if (readPos == scanPos)
		return -1;
	readPos++;
	return s_match(data + originalPos, length, ltable);
}

BUFFER_INL
(char*) append_mime(u8 mimeIndex) {
	static const u8 mimeStrings[][32] = MIME_STRINGS;

	const u8 *str = mimeStrings[mimeIndex];
	const usize length = *str;
	char *optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, str + 1, 24);
	writePos += length;
	return optr;
}
