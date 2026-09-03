#pragma once
#include "Buffer.hpp"

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

BUFFER_INL
(char*) append_digit10(usize number) {
	const usize maxLength = 24;
	char buffer[maxLength * 2];
	Span digit = fn::itoa10(number, buffer + maxLength, 24);

	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, digit.ptr, maxLength);
	writePos += digit.size;
	return optr;
}

BUFFER_INL
(char*) append_digit16(usize number) {
	const usize maxLength = 16;
	char buffer[maxLength * 2];
	Span digit = fn::itoa16(number, buffer + maxLength, 16);

	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, digit.ptr, maxLength);
	writePos += digit.size;
	return optr;
}

// Should be impossible for dst buffer to not fit
// TODO: Might remove MIN3 and have it overflow to guarantee behavior
BUFFER_INL
(usize) append_buffer(Buffer &src, usize length) {
	usize remainingSrc = src.writePos - src.readPos;	// How many bytes remain unread
	usize remainingDst = sizeof(data) - writePos;	// How many bytes are free in the buffer
	usize appendLength = MIN3(length, remainingSrc, remainingDst);

	MEMCPY(data + writePos, src.data + src.readPos, appendLength);
	src.readPos += appendLength;
	src.scanPos = (src.scanPos >= src.readPos) ? src.scanPos : src.readPos;
	writePos += appendLength;
	return appendLength;
}

BUFFER_INL
(char*) append_url_component(const char *ptr, usize length) {
	static const u8 hex[] = "0123456789ABCDEF";
	static u8 lut[2][4] = {{0, 0, 0, 1}, {'%', 0, 0, 3}};

	for (usize index = 0; index < length; index++) {
		const u8 value = (u8)ptr[index];
		const u8 lutIndex = g_asciiLut[value] <= ASCII_URL_VALID;
		lut[0][0] = value;
		lut[1][1] = hex[value >> 4];
		lut[1][2] = hex[value & 15];
		append_inline<3>((char*)lut[lutIndex], lut[lutIndex][3]);	// there has to be a better way to index the length,
															// given that it is only two possible states
	}
}

BUFFER_INL
(char*) append_html(char *ptr, usize length) {
	u8 lengthLut[6] = {5, 4, 4, 6, 5, 1};
	static char strLut[6][8] = {"&amp;", "&lt;", "&gt;", "&quot;", "&#39;", "\0"};

	for (usize index = 0; index < length; index++) {
		u8 c = MAX(6, g_asciiLut[(u8)ptr[index]] - ASCII_HTML_VALID);
		strLut[5][0] = ptr[index + 1];
		append_inline<6>(strLut[c], lengthLut[c]);	// Up to 8 bytes overflow is safe
	}
}
