#pragma once
#include "Request.hpp"
#include "core.hpp"
#include "http/Request_helpers.hpp"

// Split path/query at the first ?. V
// Reject whitespace and control characters. V
// Reject malformed encodings such as %, %2, or %GG. V
// Reject decoded NUL bytes such as %00. V
// Normalize or reject . and .. before filesystem access.
// Ensure the final resolved filesystem path remains inside the configured root.
// Do not blindly convert %2F into /; that can change path structure.
// Apply a request-target length limit and return 414 URI Too Long when exceeded
// Needs to validate the target based on above reqs
inline i32 HTTP::Request::parseTarget(const char *str, const char *end) {
	if (str >= end)	// REVIEW: str < end is guaranteed at function call
		return -1;

	const char *p = str;
	const char *questionMark = NULL;

	while (p < end) {
		if(*p <= 32)
			return -1;
		if(*p == '?') {
			if(!questionMark)
				questionMark = p;
			else
				return -1;
		}
		if(*p == '%' && p + 2 < end) { // REVIEW: p + 2 is not needed because \r\n is guaranteed to exist after end ptr
			if(!IS_DIGIT(*(p+1)) || !IS_DIGIT(*(p+2))	// REVIEW: (personal preference) but i think something like p[1] and p[2] looks cleaner
				||(*(p+1) == '0' && *(p+2) == '0'))
				return -1;
		}
		p++;
	}
	vars.path.index = str - (const char *)buffer;
	if (questionMark) {
		vars.path.size = questionMark - str;
		vars.query.index = (questionMark + 1) - (const char *)buffer;
		vars.query.size = end - (questionMark + 1);
	}
	else
		vars.path.size = end - str;
	return 0;
}

inline i32 HTTP::Request::parseFirstLine(usize length) {
	const char *str = (const char*) buffer;
	const char *end = str + length;

	if (length < 14)
		return -1;	// ERROR: Bad request "GET / HTTP/1.0" shortest possible
	if (MEMCMP_BUILTIN(str, "GET ", 4) == 0) {
		type |= HTTP::Attributes::METHOD_GET;	// TODO: create enum
		str += 4;
	}
	else if (MEMCMP_BUILTIN(str, "POST ", 5) == 0) {
		type |= HTTP::Attributes::METHOD_POST;	// TODO: create enum
		str += 5;
	}
	else if (MEMCMP_BUILTIN(str, "DELETE ", 7) == 0) {
		type |= HTTP::Attributes::METHOD_DELETE;	// TODO: create enum
		str += 7;
	}
	else
		return -1;	// ERROR: Invalid method

	const char *arg = str;
	str = end - 10;
	if (str - arg > 4096)	// TODO: Fix mixup
		return -1;
	if (MEMCMP_BUILTIN(str, " HTTP/1.", 8) == 0	&& (str[8] == '0' || str[8] == '1'))
		type |= str[8] == '1' ? (HTTP::Attributes::VERSION) : 0;	// REFACTOR: a bit confusing
	else
		return -1; // ERROR: Invalid version, TODO: what happens to the class once it is recognized as bad?

	i32 rvalue = parseTarget(arg, str);	// TODO: meaningful return
	if (rvalue < 0)
		return rvalue;

	return 0;	// No problems (YET, return code for success only happens when finally executing the method)
}

inline i32 HTTP::Request::parseLine(usize length) {
	if (line_count == 0)
		return parseFirstLine(length);

	const char *str = (char*) buffer + readOffset;
	const char *end = str + length;

	if (s_compareCase(str, end, "host:", 5) == true) {
		if (type & HTTP::Attributes::HOST)
			return -1;	// ERROR: Multiple hosts
		if (s_compareCase(str, end, "localhost", 9) == false)
			return -1;	// ERROR: Invalid host
		s_compareCase(str, end, ":8080", 5);
		type |= HTTP::Attributes::HOST;
	}
	else if (s_compareCase(str, end, "content-length:", 15) == true) { // needs length checks, or could pad
		if ((type & HTTP::Attributes::CHUNKED) || requestSize != SIZE_MAX)
			return -1; // ERROR: bad request, transfer method had already been set

		requestSize = s_readDigits(str, end);
		if (requestSize == SIZE_MAX)
			return -1;	// ERROR: Garbage after request
	}
	else if (s_compareCase(str, end, "transfer-encoding:", 18) == true) { // TODO: what if its empty?
		if ((type & HTTP::Attributes::CHUNKED) || requestSize != SIZE_MAX)
			return -1; // ERROR: bad request, transfer method had already been set

		if (s_compareCase(str, end, "chunked", 7) == false)
			return -1; // ERROR: bad request, transfer encoding isnt chunked
		type |= HTTP::Attributes::CHUNKED;	// TODO: get proper enum for bitfield
	}
	if (str != end)
		return -1; // ERROR: bad request, garbage after field value
	return 0;
}
