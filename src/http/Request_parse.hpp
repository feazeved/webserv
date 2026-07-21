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

	const char *str = (char*) buffer.data + readOffset;
	const char *end = str + length;

	if (s_compareCase(str, end, "host:", 5) == 0)
	{
		// Insert check here to see if host was already resolved
		return parseHost(str, end);
	}
	else if (s_compareCase(str, end, "content-length:", 15) == 0)	// needs length checks, or could pad
	{
		if (type >= 128 || requestSize != SIZE_MAX)
			return -1; // bad request, transfer method had already been set

		requestSize = s_readDigits(str, end);
		if (requestSize == SIZE_MAX || str != end)
			return -1;	// Garbage after request
	}
	else if (s_compareCase(str, end, "transfer-encoding:", 18) == 0)	// TODO: what if its empty?
	{
		if (type >= 128 || requestSize != SIZE_MAX)
			return -1; // bad request, transfer method had already been set

		if (s_compareCase(str, end, "chunked", 7) != 0)
			return -1; // bad request, transfer encoding isnt chunked

		if (str != end)
			return -1; // bad request, garbage after field value
		type |= 128;	// TODO: get proper enum for bitfield
	}
	return 0;
}
