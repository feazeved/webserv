#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "core.hpp"

// (Reentrant) Reading state for the header, returns true when finished parsing the header
i8 parse_header(i32 fd, usize bytes, u32 events) {
	i8 rvalue = client.read(fd, bytes, events);
	if (rvalue < 0)
		return -1;	// ERROR: Failed reading

	u32 lineEnd = client.find_line_end();
	while (lineEnd != UINT32_MAX) {
		char *ptr = (char *) client.data;
		parseLine(ptr + lineIndex, ptr + lineEnd, lineCount);
		lineCount++;
		lineIndex = lineEnd;
		lineEnd = client.find_line_end();
		if (lineEnd == 0 && type & HTTP::Attributes::DONE)
			return preValidate();
	}
	return 0;	// Actually should return something more useful like request status (processing, etc)
}

i32 parseTarget(char *str, char *end) {
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
	vars.path.index = str - (const char *)client.data;
	if (questionMark) {
		vars.path.size = questionMark - str;
		vars.query.index = (questionMark + 1) - (const char *)client.data;
		vars.query.size = end - (questionMark + 1);
	}
	else
		vars.path.size = end - str;
	return 0;
}

i32 parseFirstLine(char *str, char *end) {
	if (end - str < 14)
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

	char *arg = str;
	str = end - 9;
	if (str - arg > 4096)	// TODO: Fix mixup and magic number
		return -1;
	if (MEMCMP_BUILTIN(str, "HTTP/1.1", 8) != 0)
		return -1; // ERROR: Invalid version, TODO: what happens to the class once it is recognized as bad?

	i32 rvalue = parseTarget(arg, str);	// TODO: meaningful return
	if (rvalue < 0)
		return rvalue;
	return 0;	// No problems (YET, return code for success only happens when finally executing the method)
}

i32 parseLine(char *str, char *end, u32 lineCount) {
	if (lineCount == 0)
		return parseFirstLine(str, end);

	if (s_compare_case(str, end, "host:", 5) == true) {
		if (type & HTTP::Attributes::HOST)
			return -1;	// ERROR: Multiple hosts
		if (s_compare_case(str, end, "localhost", 9) == false)
			return -1;	// ERROR: Invalid host
		s_compare_case(str, end, ":8080", 5);
		type |= HTTP::Attributes::HOST;
	}
	else if (s_compare_case(str, end, "transfer-encoding:", 18) == true) { // TODO: what if its empty?
		if ((type & HTTP::Attributes::CHUNKED) || requestSize != SIZE_MAX)
			return -1; // ERROR: bad request, transfer method had already been set
		if (s_compare_case(str, end, "chunked", 7) == false)
			return -1; // ERROR: bad request, transfer encoding isnt chunked
		type |= HTTP::Attributes::CHUNKED;	// TODO: get proper enum for bitfield
	}
	else if (s_compare_case(str, end, "content-length:", 15) == true) { // needs length checks, or could pad
		if ((type & HTTP::Attributes::CHUNKED) || requestSize != SIZE_MAX)
			return -1; // ERROR: bad request, transfer method had already been set
		requestSize = s_read_digits(str);
		if (requestSize == SIZE_MAX)
			return -1;	// ERROR: Garbage after request
		return 0;
	}

	if (str != end)
		return -1; // ERROR: bad request, garbage after field value
	return 0;
}
