#pragma once
#include "Buffer.hpp"

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
	result.ptr = (char*)data + readPos;
	result.size = valueEnd - readPos;
	readPos = scanPos;
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
