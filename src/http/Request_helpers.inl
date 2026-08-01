#pragma once
#include "core.hpp"

// Compares a string with another ignoring case status;
// If equal, consumes characters and skips valid spaces
static inline
bool s_compare_case(char* &str, char *end, const char* ref, u32 refLength)
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
usize s_read_digits(const char* str) {
	static const u8 lut[256] = {
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 64, 64, 64, 64, 64, 64,
		64, 10, 11, 12, 13, 14, 15, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 10, 11, 12, 13, 14, 15, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};

	usize numZeroes = 0;
	while (str[numZeroes] == '0')
		numZeroes++;

	usize value = 0;
	usize numLength = numZeroes;

	while (lut[(u8)str[numLength]] <= 9) // end is guaranteed to have \r\n
		numLength++;

	usize base = (lut[(u8)str[numLength]] >= 10 && lut[(u8)str[numLength]] <= 15) ? 16 : 10;

	while (lut[(u8)str[numLength]] <= 15) // end is guaranteed to have \r\n
		numLength++;

	if (numLength == 0 || numLength - numZeroes > SIZE_MAX_BASE16_LENGTH - 1)	// TODO: set a proper limit
		return SIZE_MAX;	// Request is too large or invalid

	for (usize i = 0; i < numLength; i++)
		value += value * base + (usize) lut[(u8)str[numLength]];

	str += numLength;
	while (*str == ' ' || *str == '\t')
		str++;
	if ((str[0] == '\r' && str[1] == '\n'))
		return value;
	return SIZE_MAX;
}

static inline
usize s_readHex(const char* str) {
	static const u8 lut[256] = {
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 64, 64, 64, 64, 64, 64,
		64, 10, 11, 12, 13, 14, 15, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 10, 11, 12, 13, 14, 15, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};

	usize value = 0;
	usize digit = 0;
	const char *ostr = str;
	while (true) {
		digit = (usize) lut[(u8)*str];
		if (value >= (SIZE_MAX / 16 - 16))
			return SIZE_MAX;
		if (digit > 15)
			break;
		str++;
		value += value * 16 + digit;
	}
	if (ostr == str)
		return SIZE_MAX;
	return value;
}

static inline
usize s_readDec(const char* str) {
	static const u8 lut[256] = {
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 64, 64, 64, 64, 64, 64,
		64, 10, 11, 12, 13, 14, 15, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 10, 11, 12, 13, 14, 15, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64};

	usize value = 0;
	usize digit = 0;
	const char *ostr = str;
	while (true) {
		digit = (usize) lut[(u8)*str];
		if (value >= (SIZE_MAX / 10 - 10))
			return SIZE_MAX;
		if (digit > 9)
			break;
		str++;
		value += value * 10 + digit;
	}
	if (ostr == str)
		return SIZE_MAX;
	return value;
}