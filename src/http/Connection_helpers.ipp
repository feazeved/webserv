#pragma once
#include "core.hpp"
#include "fcntl.h"

// Compares a string with another ignoring case status;
// If equal, consumes characters and skips valid spaces
// TODO: fix this shit
static inline
bool s_compare_case(char* &str, char *end, const char* ref, u32 refLength)
{
	if (str + refLength > end)
		return false;

	u32 i = 0;
	while (i < refLength)
	{
		u8 c = g_asciiLut [(u8) ref[i]];
		if (g_asciiLut[(u8) str[i]] != c)
			return false;
		i++;
	}

	str += i;
	while (*str == ' ' || *str == '\t')
		str++;
	return true;
}

static inline
bool s_skip_spaces(char *&str) {
	while ((*str == ' ' || *str == '\t'))
		str++;
	return MEMCMP(str, "\r\n", 2) == 0;
}

static inline
usize s_itoa10(usize number, char *bufferEnd) {
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

static inline
usize s_itoa16(usize number, char *bufferEnd) {
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

static inline
usize s_strtol16(const char* str) {
	usize value = 0;
	usize digit = 0;
	const char *ostr = str;
	while (true) {
		digit = (usize) g_asciiLut[(u8)*str];
		if (value >= (SIZE_MAX / 16 - 16))
			return SIZE_MAX;
		if (digit > 15)
			break;
		str++;
		value = value * 16 + digit;
	}
	if (ostr == str || MEMCMP(str, "\r\n", 2) != 0)	// TODO: Check if not too strict
		return SIZE_MAX;
	return value;
}

static inline
usize s_strtol10(const char* str) {
	usize value = 0;
	usize digit = 0;
	const char *ostr = str;
	while (true) {
		digit = (usize) g_asciiLut[(u8)*str];
		if (value >= (SIZE_MAX / 10 - 10))
			return SIZE_MAX;
		if (digit > 9)
			break;
		str++;
		value = value * 10 + digit;
	}
	if (ostr == str || MEMCMP(str, "\r\n", 2) != 0)
		return SIZE_MAX;
	return value;
}

static inline
bool s_set_noblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return false;

	return true;
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
static inline
isize s_match_field(char* &ptr, char* end) {
	static const char fieldTable[][32] = 
	{"status", "location", "transfer-encoding", "content-length"};	
	static const usize fieldCount = ARRAY_SIZE(fieldTable);
	char *optr = ptr;

	while (ptr < end && *ptr != ':')
		ptr++;
	usize length = (usize)(ptr - optr);
	if (length >= 32 || *ptr != ':')
		return (*ptr != ':') ? -1 : 0;
	ptr++;
	char buffer[64];
	MEMCPY_INLINE(buffer, optr, 32);
	for (usize i = 0; i < 32; i++)
		buffer[i] |= 32;
	MEMSET_INLINE(buffer + length, 0, 32);

	for (usize i = 0; i < fieldCount; i++) {
		if (MEMCMP(fieldTable[i], buffer, 32) == 0)
			return (isize)i + 1;
	}
	return 0;
}
