#pragma once
#include "Request.hpp"

inline i32 HTTP::Request::parseFirstLine(usize length)
{
	const char *str = (const char*) buffer.data;
	const char *end = str + length;

	if (length < 14)
		return -1;	// Bad request "GET / HTTP/1.0" shortest possible
	if (std::memcmp(str, "GET ", 4) == 0)
	{
		type |= 1;	// TODO: create enum
		str += 4;
	}
	else if (std::memcmp(str, "POST ", 5) == 0)
	{
		type |= 2;	// TODO: create enum
		str += 5;
	}
	else if (std::memcmp(str, "DELETE ", 7) == 0)
	{
		type |= 4;	// TODO: create enum
		str += 7;
	}
	else
		return -1;	// Invalid method

	const char *arg = str;
	while (str < end && *str != ' ')
		str++;
	i32 rvalue = parseArg(arg, str);
	if (rvalue < 0)
		return rvalue;

	if (str + 9 == end 
		&& std::memcmp(str, " HTTP/1.", 8) == 0
		&& (str[8] == '0' || str[8] == '1'))
	{
		type |= (str[8] - '0') << 3;	// TODO: get proper enum bitfield representation
	}
	else
		return -1; // Invalid version, TODO: what happens to the class once it is recognized as bad?
	return 0;	// No problems (YET, return code for success only happens when finally executing the method)
}

inline i32 HTTP::Request::parseLine(usize length)
{
	if (line_count == 0)
		return parseFirstLine(length);

	char *str = (char*) buffer.data + readOffset;
	char *end = str + length;

	while (str < end && *str != ':')
	{
		if (*str >= 'A' && *str <= 'Z')
			*str += 32;
		str++;
	}

	if (std::memcmp(str, "host:", 5) == 0)
	{
		str += 5;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;
		// Insert check here to see if host was already resolved
		return parseHost(str, end);
	}
	else if (std::memcmp(str, "content-length:", 15) == 0)	// needs length checks, or could pad
	{
		if (type >= 128 || requestSize != UINT32_MAX)
			return -1; // bad request, transfer method had already been set
		str += 15;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;

		usize numberLength = 0;
		u32 value = 0;
		while (str[numberLength] >= '0' && str[numberLength] <= '9')
		{
			value = value * 10 + (u32)(str[numberLength] - '0');
			numberLength++;
		}
		if (numberLength == 0 || numberLength > 9)
			return -1;	// Request is too large or invalid
		str += numberLength;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;
		if (str != end)
			return -1;	// Garbage after request
	}
	else if (std::memcmp(str, "transfer-encoding:", 18) == 0)	// TODO: what if its empty?
	{
		if (type >= 128 || requestSize != UINT32_MAX)
			return -1; // bad request, transfer method had already been set
		str += 18;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;

		char *ostr = str;
		while ((*str >= 'A' && *str <= 'Z') || (*str >= 'a' && *str <= 'z'))
			*str++ |= 32;
		if (std::memcmp(ostr, "chunked", 7) != 0)
			return -1; // bad request, transfer encoding isnt chunked
		str = ostr + 7;
		while (str < end && (*str == ' ' || *str == '\t'))
			str++;
		if (str != end)
			return -1; // bad request, garbage after field value
		type |= 128;	// TODO: get proper enum for bitfield
	}
	return 0;
}
