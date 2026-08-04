#pragma once

#include "Connection.hpp"
#include "http/Connection_helpers.ipp"

namespace HTTP {

/*
Functions: 
Something that takes the config file for the server and a request, and validates:

Function receives a file path, a buffer, IO direction (read or write), number of bytes:

1) Checks if the file path exists, if the server has permission to access it 
2) Finally return an FD or -1
*/
// (Reentrant) Reading state for the header, returns true when finished parsing the header
template <usize bufferSize> inline
isize Connection<bufferSize>::parse_header(usize bytes, u32 events) {
	isize bytesRead = read_from_client(bytes, events);
	if (bytesRead < 0)
		return bytesRead;

	isize rvalue;
	while ((rvalue = clientOutput.find_line_end()) != 0) {
		u8 *lineStart = clientOutput.data + clientOutput.start;
		u8 *lineEnd = clientOutput.data + clientOutput.end;
		if (parse_line(lineStart, lineEnd) < 0)
			return error_path();	// ERROR: Invalid header
		if (rvalue == 2) {
			// Header end, call configure() and setup
		}
	}
	return 0;	// Actually should return something more useful like request status (processing, etc)
}

template <usize bufferSize> inline
isize Connection<bufferSize>::parse_target(char *str, char *end) {
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
		if(*p == '.' && p[1] == '.')
			return -1;
		if(*p == '%' && p + 2 < end) { // REVIEW: p + 2 is not needed because \r\n is guaranteed to exist after end ptr
			if(!IS_DIGIT(p[1]) || !IS_DIGIT(p[2]) // REVIEW: if im not mistaken the check here should be IS_HEX
				||(p[1] == '0' && p[2] == '0'))
				return -1;
		}
		p++;
	}
	vars.path.index = str - (const char *)clientOutput.data;
	if (questionMark) {
		vars.path.size = questionMark - str;
		vars.query.index = (questionMark + 1) - (const char *)clientOutput.data;
		vars.query.size = end - (questionMark + 1);
	}
	else
		vars.path.size = end - str;
	return 0;
}

template <usize bufferSize> inline
isize Connection<bufferSize>::parse_first_line(char *str, char *end) {
	if (end - str < 14)
		return -1;	// ERROR: Bad request "GET / HTTP/1.0" shortest possible
	if (MEMCMP(str, "GET ", 4) == 0) {
		type |= Attributes::GET;	// TODO: create enum
		str += 4;
	}
	else if (MEMCMP(str, "POST ", 5) == 0) {
		type |= Attributes::POST;	// TODO: create enum
		str += 5;
	}
	else if (MEMCMP(str, "DELETE ", 7) == 0) {
		type |= Attributes::DELETE;	// TODO: create enum
		str += 7;
	}
	else
		return -1;	// ERROR: Invalid method

	char *arg = str;
	str = end - 9;
	if (str - arg > 4096)	// TODO: Fix mixup and magic number
		return -1;
	if (MEMCMP(str, "HTTP/1.1", 8) != 0)
		return -1; // ERROR: Invalid version, TODO: what happens to the class once it is recognized as bad?

	i32 rvalue = parse_target(arg, str);	// TODO: meaningful return
	if (rvalue < 0)
		return rvalue;
	return 0;	// No problems (YET, return code for success only happens when finally executing the method)
}

template <usize bufferSize> inline
isize Connection<bufferSize>::parse_line(char *str, char *end) {
	if (s_compare_case(str, end, "host:", 5) == true) {
		if (type & Attributes::HOST)
			return -1;	// ERROR: Multiple hosts
		if (s_compare_case(str, end, "localhost", 9) == false)
			return -1;	// ERROR: Invalid host
		s_compare_case(str, end, ":8080", 5);
		type |= Attributes::HOST;
	}
	else if (s_compare_case(str, end, "transfer-encoding:", 18) == true) { // TODO: what if its empty?
		if ((type & Attributes::CHUNKED) || bodySize != SIZE_MAX)
			return -1; // ERROR: bad request, transfer method had already been set
		if (s_compare_case(str, end, "chunked", 7) == false)
			return -1; // ERROR: bad request, transfer encoding isnt chunked
		type |= Attributes::CHUNKED;	// TODO: get proper enum for bitfield
	}
	else if (s_compare_case(str, end, "content-length:", 15) == true) { // needs length checks, or could pad
		if ((type & Attributes::CHUNKED) || bodySize != SIZE_MAX)
			return -1; // ERROR: bad request, transfer method had already been set
		bodySize = s_strtol10(str);
		if (bodySize == SIZE_MAX)
			return -1;	// ERROR: Garbage after request
		return 0;
	}

	if (str != end)
		return -1; // ERROR: bad request, garbage after field value
	return 0;
}
}
