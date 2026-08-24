#pragma once
#include "Request.hpp"

namespace HTTP {

REQUEST_INL
(isize) parse_line(HTTP_Buffer &src, VirtualServer* cfg, usize lineLength) {
	if (lineLength < 2 || lineLength >= 8000) {
		status = lineLength < 2 ? Status::i400 : Status::i431;
		return -1;
	}

	u8* lineEnd = src.readPtr + lineLength;
	const isize fieldIndex = src.match_field();
	if (fieldIndex < 0 || !src.skip_spaces())	// Reject empty values
		goto Error;
	switch (fieldIndex) {
		default:
			src.readPtr = lineEnd;
			break;

		case Field::TRANSFER_ENCODING:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				goto Error; // ERROR: bad request, transfer method had already been set
			if (src.strcasecmp("chunked") == false)
				goto Error; // ERROR: bad request, transfer encoding isnt chunked
			options |= Options::CHUNKED_LENGTH;
			break;

		case Field::CONTENT_LENGTH:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				goto Error; // ERROR: bad request, transfer method had already been set
			bodySize = src.strtol10();
			if (bodySize == SIZE_MAX)
				goto Error;
			if (bodySize > cfg->maxBodySize) {
				status = Status::i413;
				return -1;
			}
			options |= Options::FIXED_LENGTH;
			break;

		case Field::HOST:
			if (options & Options::HOST)
				goto Error;	// ERROR: Multiple hosts
			if (src.strcasecmp("localhost") == false)
				goto Error;
			src.strcasecmp(":8080");
			options |= Options::HOST;
			break;
		case Field::COOKIES:
			while ((lineEnd[-1] == ' ' || lineEnd[-1] == '\t'))
				lineEnd--;
			cookies.index = (u16)(src.readPtr - src.data);
			cookies.size = (u16)(lineEnd - src.readPtr);
			break;
	}

	if (src.skip_spaces())
		goto Error;
	src.readPtr = src.scanPtr;
	return fieldIndex;

Error:
	status = Status::i400;
	return -1;
}

/*
	CGI output is server-controlled, so this path performs only the inexpensive
	structural checks needed before forwarding the line.
*/
REQUEST_INL
(isize) parse_cgi_line(HTTP_Buffer &src, HTTP_Buffer &dst) {
	const char* const field = (char*)src.readPtr;
	const char* const lineEnd = (char*)src.scanPtr - 2;
	const usize totalLength = (usize)(lineEnd - (char*)src.readPtr);

	const isize fieldIndex = src.match_field();
	if (fieldIndex <= 0) {
		if (fieldIndex < 0)
			status = Status::i500;
		else
			dst.append(field, totalLength);
		src.readPtr = src.scanPtr;
		return fieldIndex;
	}

	if (fieldIndex == Field::STATUS) {
		status = (char*) src.readPtr;
		isize rvalue = status.is_valid() == true ? 0 : -1;
		if (rvalue == -1)
			status = Status::i500;	// CGI output an invalid status, should be server error
		StringView str = status.status_str();
		dst.prepend(str.ptr, str.length);
		return rvalue;
	}

	dst.append(field, totalLength);
	src.readPtr = src.scanPtr;
	return fieldIndex;
}

}
