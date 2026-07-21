#pragma once
#include "Request.hpp"

// Compares a string with another ignoring case status;
// If equal, consumes characters and skips valid spaces
static inline
bool s_compareCase(const char* &str, const char *end, const char* ref, u32 refLength)
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
usize s_readDigits(const char* &str, const char *end)
{
	while (*str == '0')
		str++;

	usize value = 0;
	const char *ostr = str;

	while (*str >= '0' && *str <= '9') // end is guaranteed to have \r\n
	{
		value = value * 10 + (usize)(*str - '0');
		str++;
	}

	if (ostr == str || (usize) (str - ostr) > SIZE_MAX_BASE10_LENGTH - 1)	// TODO: set a proper limit
		return SIZE_MAX;	// Request is too large or invalid
	while (str < end && (*str == ' ' || *str == '\t'))
		str++;
	return value;
}
