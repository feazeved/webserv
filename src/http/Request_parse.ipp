#pragma once
#include "Request.hpp"

namespace HTTP {

REQUEST_INL
(isize) parse_header(Cursor &src) {
	isize rvalue;
	while ((rvalue = src.find_line_end()) != 0) {
		if (parse_line(src) < 0)
			return -1;	// ERROR: Invalid header
		if (rvalue == 2)
			return 1;
	}
	return 0;
}

// TODO: set status here
REQUEST_INL
(isize) parse_line(Cursor &src) {
	const usize lineLength = (usize)(src.lineEnd - src.linePtr);
	if (lineLength < 2 || lineLength >= 8192)
		return -1;

	isize fieldIndex = src.match_field();
	switch (fieldIndex) {
		case Field::INVALID:
			return -1;

		default:
			return 0;

		case Field::LOCATION:
			
			break;

		case Field::TRANSFER_ENCODING:
			if ((options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH)))
				return -1; // ERROR: bad request, transfer method had already been set
			if (src.strcasecmp("chunked") == false)
				return -1; // ERROR: bad request, transfer encoding isnt chunked
			options |= Options::CHUNKED_LENGTH;
			break;

		case Field::CONTENT_LENGTH:
			if ((options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH)))
				return -1; // ERROR: bad request, transfer method had already been set
			bodySize = src.strtol10();
			if (bodySize == SIZE_MAX)
				return -1;	// ERROR: not a number or too large
			break;

		case Field::HOST:
			if (options & Options::HOST)
				return -1;	// ERROR: Multiple hosts
			if (src.strcasecmp("localhost") == false)
				return -1;
			src.strcasecmp(":8080");
			options |= Options::HOST;
			break;
	}

	if (src.skip_spaces())
		return -1;	// ERROR: bad request, garbage after field value
	return fieldIndex;	// Positive values mean something was matched, 0 means unknown
}

// NOTES: i think the call here is because CGI output is server controlled, 
// we perform little error checking and presume the output is correct. 
// Only quick sanity checks
REQUEST_INL
(isize) parse_cgi_line(Cursor &src, Cursor &dst) {
	const u8 *field = src.linePtr;
	const usize totalLength = src.lineEnd - src.linePtr;

	isize fieldIndex = src.match_field();
	if (fieldIndex <= 0) {
		if (fieldIndex < 0)
			status = Status::i500;
		return fieldIndex;
	}

	if (fieldIndex == Field::STATUS) {
		status = (char*) src.linePtr;
		isize rvalue = status.is_valid() == true ? 0 : -1;
		if (rvalue == -1)
			status = Status::i500;	// CGI output an invalid status, should be server error
		const u8 *str = (const u8*) status.c_str();
		usize length = status.size();
		dst.insert(str, length, 256 - length);
		return rvalue;
	}
	else {
		dst.append(field, totalLength);
	}
	return fieldIndex;
}

// HTTP NAMESPACE END
}
