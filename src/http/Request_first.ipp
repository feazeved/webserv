#pragma once
#include "Request.hpp"

namespace HTTP {

static inline
bool s_checkType(const std::string &method, std::vector<std::string>::iterator &mit, std::vector<std::string>::iterator &end){
	for(; mit != end; mit++)
		if(method == *mit)
			return true;
	return false;
}

static inline
bool s_checkLocation(const char *path, usize pathLength, ServerConfig* cfg){
	// bool found = false;
	// std::vector<Location>::iterator it = cfg->locations.begin();

	// for(; it != cfg->locations.end(); it++)
	// {
	// 	// if (MEMCMP(path, it->path.c_str(), it->path.size()) == 0 && path.size == it->path.size())
	// 	// {
	// 	// 	found = true;
	// 	// 	break ;
	// 	// }
	// }

	// if (found)
	// {
	// 	// Changing this
	// 	std::string method;
	// 	std::vector<std::string>::iterator begin = it->methods.begin();
	// 	std::vector<std::string>::iterator end = it->methods.end();

	// 	// if (type & (Attributes::GET))
	// 	// 	method = "GET ";
	// 	// else if (type & (Attributes::POST))
	// 	// 	method = "POST ";
	// 	// else if (type & (Attributes::DELETE))
	// 	// 	method = "DELETE ";
	// 	// else
	// 	// 	return false;

	// 	return (s_checkType(method, begin, end));
	// }
	// return false;
}

REQUEST_INL
(isize) parse_target(Buffer<bufferSize> &src, ServerConfig* cfg) {
	// const char *p = str;
	// const char *questionMark = NULL;

	// while (p < end) {
	// 	if (*p <= 32)
	// 		return -1;
	// 	if (*p == '?') {
	// 		if(!questionMark)
	// 			questionMark = p;
	// 		else
	// 			return -1;
	// 	}
	// 	if (*p == '.' && p[1] == '.')
	// 		return -1;
	// 	if (*p == '%' && ((!IS_DIGIT(p[1]) || !IS_DIGIT(p[2]) || (p[1] == '0' && p[2] == '0'))))	// TODO: should be hex i was wrong
	// 			return -1;
	// 	p++;
	// }

	// path.index = str - (const char *)clientOutput.data;
	// if (questionMark) {
	// 	path.size = questionMark - str;
	// 	query.index = (questionMark + 1) - (const char *)clientOutput.data;
	// 	query.size = end - (questionMark + 1);
	// }
	// else
	// 	path.size = end - str;

	// //check method/location

	// if(!s_checkLocation(path, cfg))
	// 	return -1;

	// return 0;
}

REQUEST_INL
(isize) parse_first_line(Buffer<bufferSize> &src, ServerConfig* cfg) {
	const usize lineLength = src.lineEnd - src.linePtr;
	if (lineLength < 14 || lineLength >= 8192)
		return -1;	// ERROR: Bad request "GET / HTTP/1.0" shortest possible

	if (src.strcmp("GET"))
		type |= Attributes::GET;
	else if (src.strcmp("POST"))
		type |= Attributes::POST;
	else if (src.strcmp("DELETE"))
		type |= Attributes::DELETE;
	else
		return -1;	// ERROR: Invalid method

	if (MEMCMP(src.lineEnd - 9, "HTTP/1.1", 8) != 0)
		return -1; // ERROR: Invalid version

	i32 rvalue = parse_target(src, cfg);	// TODO: meaningful return
	if (rvalue < 0)
		return rvalue;
	return 0;	// No problems (YET, return code for success only happens when finally executing the method)
}

// HTTP NAMESPACE END
}
