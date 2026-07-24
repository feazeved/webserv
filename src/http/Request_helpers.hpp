#pragma once
#include "core.hpp"

// Compares a string with another ignoring case status;
// If equal, consumes characters and skips valid spaces
static inline
bool s_compare_case(const char* &str, const char *end, const char* ref, u32 refLength)
{
	if (str + refLength > end)
		return false;

	u32 i = 0;
	while (i < refLength)
	{
		if (str[i] != ref[i] && (ref[i] >= 'A' && ref[i] <= 'Z' && (str[i] | 32) != ref[i]))
			return false;
		i++;
	}

	str += i;
	while (str < end && (*str == ' ' || *str == '\t'))
		str++;
	return true;
}

// basic atoi
// consumes characters and skips valid spaces
static inline
usize s_read_digits(const char* &str, const char *end)
{
	static const i8 lut[256] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

	usize numZeroes = 0;
	while (str[numZeroes] == '0')
		numZeroes++;

	usize value = 0;
	usize numberLength = numZeroes;
	usize base = 10;

	while (lut[(u8)str[numberLength]] >= 0) // end is guaranteed to have \r\n
	{
		if (lut[(u8)str[numberLength]] > 9)
			base = 16;
		numberLength++;
	}

	if (numberLength == 0 || numberLength - numZeroes > SIZE_MAX_BASE16_LENGTH - 1)	// TODO: set a proper limit
		return SIZE_MAX;	// Request is too large or invalid

	for (usize i = 0; i < numberLength; i++)
		value += value * base + (usize) lut[(u8)str[numberLength]];

	str += numberLength;
	while (str < end && (*str == ' ' || *str == '\t'))
		str++;
	return value;
}
