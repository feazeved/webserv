#pragma once
#include "Request.hpp"

namespace HTTP {

// TODO: Add a check for line length here if it makes sense
REQUEST_INL
(isize) parse_header(Cursor &src, ServerConfig* cfg) {
	isize rvalue;
	while ((rvalue = src.find_line_end()) != 0) {
		if (parse_line(src, cfg) < 0)
			return -1;	// ERROR: Invalid header
		if (rvalue == 2)
			return validate_header();
	}
	return 0;
}

REQUEST_INL
(isize) validate_header() {
	const bool isBodyMethod = mode & (Mode::POST | Mode::CGI);
	const bool encodingSet = !(options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH));

	if (status.is_set())
		return -1;	// An error caused early interruption

	if ((mode & 0xF) == 0)
		return -1;	// TODO: Method not set, should be impossible. Remove in future

	if ((mode & Options::HOST) == 0)
		return -1;	// Host not set

	if (isBodyMethod && !encodingSet)
		return -1;	// Transfer encoding not set

	if (!isBodyMethod && encodingSet)
		return -1;	// Encoding set for non-body methods

	return 1;
}

REQUEST_INL
(isize) parse_line(Cursor &src, ServerConfig* cfg) {
	const usize lineLength = (usize)(src.lineEnd - src.readPtr);
	if (lineLength < 2 || lineLength >= 8192) {	// TODO: Fix magic numbers
		status = Status::i401;
		return -1;
	}

	isize fieldIndex = src.match_field();
	switch (fieldIndex) {
		default:
			if (fieldIndex < 0)
				goto Error;
			return fieldIndex;

		case Field::LOCATION:
			
			break;

		case Field::TRANSFER_ENCODING:
			if ((options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH)))
				goto Error; // ERROR: bad request, transfer method had already been set
			if (src.strcasecmp("chunked") == false)
				goto Error; // ERROR: bad request, transfer encoding isnt chunked
			options |= Options::CHUNKED_LENGTH;
			break;

		case Field::CONTENT_LENGTH:
			if ((options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH)))
				goto Error; // ERROR: bad request, transfer method had already been set
			bodySize = src.strtol10();
			options |= Options::FIXED_LENGTH;
			if (bodySize > cfg->maxBodySize)
				goto Error;	// ERROR: not a number or too large
			break;

		case Field::HOST:
			if (options & Options::HOST)
				goto Error;	// ERROR: Multiple hosts
			if (src.strcasecmp("localhost") == false)
				goto Error;
			src.strcasecmp(":8080");
			options |= Options::HOST;
			break;
	}

	if (src.skip_spaces())
		goto Error;	// ERROR: bad request, garbage after field value

	return fieldIndex;	// Positive values mean something was matched, 0 means unknown

	Error:
		status = Status::i400;
		return -1;
}

// NOTES: i think the call here is because CGI output is server controlled, 
// we perform little error checking and presume the output is correct. 
// Only quick sanity checks
REQUEST_INL
(isize) parse_cgi_line(Cursor &src, Cursor &dst) {
	const u8 *field = src.readPtr;
	const usize totalLength = (usize)(src.lineEnd - src.readPtr);

	isize fieldIndex = src.match_field();
	if (fieldIndex <= 0) {
		if (fieldIndex < 0)
			status = Status::i500;
		return fieldIndex;
	}

	if (fieldIndex == Field::STATUS) {
		status = (char*) src.readPtr;
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
