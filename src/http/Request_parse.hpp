#pragma once
#include "Request.hpp"
#include "core.hpp"
#include <cctype>
#include <cstring>

// Split path/query at the first ?. V
// Reject whitespace and control characters. V
// Reject malformed encodings such as %, %2, or %GG. V
// Reject decoded NUL bytes such as %00. V
// Normalize or reject . and .. before filesystem access.
// Ensure the final resolved filesystem path remains inside the configured root.
// Do not blindly convert %2F into /; that can change path structure.
// Apply a request-target length limit and return 414 URI Too Long when exceeded
// Needs to validate the target based on above reqs
inline i32 HTTP::Request::parseTarget(const char *str, const char *end)
{
	if (str >= end)
		return -1;

	const char *p = str;
	const char *questionMark = NULL;

	while (p < end)
	{
		if(*p <= 32)
			return -1;
		if(*p == '?')
		{
			if(!questionMark)
				questionMark = p;
			else
				return -1;
		}
		if(*p == '%' && p + 2 < end)
		{
			if(!std::isxdigit(*(p+1)) || !std::isxdigit(*(p+2))
				||(*(p+1) == '0' && *(p+2) == '0'))
				return -1;
		}
		p++;
	}
	pathOffset = str - (const char *)buffer.data;
	if (questionMark)
	{
		pathSize = questionMark - str;
		queryOffset = (questionMark + 1) - (const char *)buffer.data;
		querySize = end - (questionMark + 1);
	}
	else
		pathSize = end - str;
	return 0;
}

inline i32 HTTP::Request::parseFirstLine(usize length)
{
	const char *str = (const char*) buffer.data;
	const char *end = str + length;

	if (length < 14)
		return -1;	// Bad request "GET / HTTP/1.0" shortest possible
	if (MEMCMP_BUILTIN(str, "GET ", 4) == 0)
	{
		type |= 1;	// TODO: create enum
		str += 4;
	}
	else if (MEMCMP_BUILTIN(str, "POST ", 5) == 0)
	{
		type |= 2;	// TODO: create enum
		str += 5;
	}
	else if (MEMCMP_BUILTIN(str, "DELETE ", 7) == 0)
	{
		type |= 4;	// TODO: create enum
		str += 7;
	}
	else
		return -1;	// Invalid method

	const char *arg = str;
	while (str < end && *str != ' ')
		str++;
	i32 rvalue = parseTarget(arg, str);	// TODO: meaningful return
	if (rvalue < 0)
		return rvalue;

	if (str + 9 == end 
		&& MEMCMP_BUILTIN(str, " HTTP/1.", 8) == 0
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
