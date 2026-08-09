#pragma once

#include "Connection.hpp"
#include "http/Connection_helpers.ipp"
#include <iterator>
#include <vector>

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
		char *lineStart = clientOutput.data + clientOutput.start;
		char *lineEnd = clientOutput.data + clientOutput.end;
		if (parse_line(lineStart, lineEnd) < 0)
			return error_path();	// ERROR: Invalid header
		if (rvalue == 2) {
			// Header end, call configure() and setup
			headerParsed = true;
			break ;
		}
	}
	return 0;	// Actually should return something more useful like request status (processing, etc)
}

template <usize bufferSize> inline
bool Connection<bufferSize>::checkLocation(){
	bool found = false;
	std::vector<Location>::iterator it = cfg->locations.begin();

	for(; it != cfg->locations.end(); it++)
	{
		if (MEMCMP(vars.path.index, it->path.c_str(), it->path.size()) == 0 && vars.path.size == it->path.size())
		{
			found = true;
			break ;
		}
	}

	if (found)
		return (it->methods & type) ? true : false;
	return false;
}

// TODO: parsing should process the data not just store it
// Alex: Lets have this save an enum to extension type. Example:
// enum ContentType : u8 {
// 	Html,
// 	Css,
// 	JavaScript,
// 	Json,
// 	Png,
// 	Jpeg,
// 	Gif,
// 	Text,
// };
static inline
const char* s_get_mime_type(const std::string& path) {
	usize	dot_pos = path.find_last_of('.');	// Alex: Ideally this function should be inside parse_target
												// Save an enum to the file type inside the class, might be relevant for other stuff
	if (dot_pos == std::string::npos)
		return "application/octet-stream";
	std::string extension = path.substr(dot_pos + 1);
	if (extension == "html" || extension == "htm")	return "text/html";
	if (extension == "css")                 		return "text/css";
	if (extension == "js")                  		return "application/javascript";
	if (extension == "json")                 		return "application/json";
	if (extension == "png")                  		return "image/png";
	if (extension == "jpg" || extension == "jpeg")	return "image/jpeg";
	if (extension == "gif")                  		return "image/gif";
	if (extension == "txt")                  		return "text/plain";
	return "application/octet-stream";
}

template <usize bufferSize> inline
isize Connection<bufferSize>::parse_target(char *str, char *end) {
	const char *p = str;
	const char *questionMark = NULL;

	while (p < end) {
		if (*p <= 32)
			return -1;
		if (*p == '?') {
			if(!questionMark)
				questionMark = p;
			else
				return -1;
		}
		if (*p == '.' && p[1] == '.')
			return -1;
		if (*p == '%' && ((!IS_DIGIT(p[1]) || !IS_DIGIT(p[2]) || (p[1] == '0' && p[2] == '0'))))	// TODO: should be hex i was wrong
				return -1;
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

	//check method/location

	if(!checkLocation())
		return -1;

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
