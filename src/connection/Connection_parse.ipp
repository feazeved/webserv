#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) parse_first_line(usize lineLength) {
	if (lineLength < 14 || lineLength >= 8000) {
		status = lineLength < 14 ? Status::i400 : Status::i431;
		return -1;	// ERROR: Bad request "GET / HTTP/1.1" shortest possible
	}
	char *const lineEnd = recvBuffer.rptr() + lineLength;

	if (recvBuffer.strcmp("GET "))
		options |= Options::GET;
	else if (recvBuffer.strcmp("POST "))
		options |= Options::POST;
	else if (recvBuffer.strcmp("DELETE "))
		options |= Options::DELETE;
	else {
		status = Status::i501;
		return -1;
	}

	if (MEMCMP(lineEnd - 9, " HTTP/1.1", 9) != 0) {
		status = Status::i505;
		return -1;
	}
	*(lineEnd - 9) = 0;
	const isize result = validate_target();
	if (result < 0 && !status.is_set())
		status = Status::i400;
	// recvBuffer.readPos = recvBuffer.scanPtr;	// TODO: add skip spaces
	return result;
}

CONNECTION_INL
(isize) parse_line(usize lineLength) {
	if (lineLength < 2 || lineLength >= 8000) {
		status = lineLength < 2 ? Status::i400 : Status::i431;
		return -1;
	}

	char* lineEnd = recvBuffer.rptr() + lineLength;
	const isize fieldIndex = recvBuffer.match_field();
	if (fieldIndex < 0 || !recvBuffer.skip_spaces())	// Reject empty values
		goto Error;
	switch (fieldIndex) {
		default:
			recvBuffer.readPos += lineLength;
			break;

		case Field::TRANSFER_ENCODING:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				goto Error; // ERROR: bad request, transfer method had already been set
			if (recvBuffer.strcasecmp("chunked") == false)
				goto Error; // ERROR: bad request, transfer encoding isnt chunked
			options |= Options::CHUNKED_LENGTH;
			break;

		case Field::CONTENT_LENGTH:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				goto Error; // ERROR: bad request, transfer method had already been set
			bodySize = recvBuffer.strtol10();
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
			if (recvBuffer.strcasecmp("localhost") == false)
				goto Error;
			recvBuffer.strcasecmp(":8080");
			options |= Options::HOST;
			break;
		case Field::COOKIES:
			while ((lineEnd[-1] == ' ' || lineEnd[-1] == '\t'))
				lineEnd--;
			req.cookies.ptr = (char*) recvBuffer.rptr();
			req.cookies.length = (usize) (lineEnd - recvBuffer.rptr());
			break;
	}

	if (recvBuffer.skip_spaces())
		goto Error;
	// recvBuffer.readPos = recvBuffer.scanPos;
	return fieldIndex;

Error:
	status = Status::i400;
	return -1;
}

/*
	CGI output is server-controlled, so this path performs only the inexpensive
	structural checks needed before forwarding the line.
*/
CONNECTION_INL
(isize) parse_cgi_line(Buffer16 &dst) {
	const char* const field = (char*)sendBuffer.rptr();
	const char* const lineEnd = (char*)sendBuffer.sptr() - 2;
	const usize totalLength = (usize)(lineEnd - (char*)sendBuffer.rptr());

	const isize fieldIndex = sendBuffer.match_field();
	if (fieldIndex <= 0) {
		if (fieldIndex < 0)
			status = Status::i500;
		else
			dst.append(field, totalLength);
		// sendBuffer.rptr() = sendBuffer.sptr();
		return fieldIndex;
	}

	if (fieldIndex == Field::STATUS) {
		status = (char*) sendBuffer.rptr();
		isize rvalue = status.is_valid() == true ? 0 : -1;
		if (rvalue == -1)
			status = Status::i500;	// CGI output an invalid status, should be server error
		dst.prepend(status.status_str());
		return rvalue;
	}

	dst.append(field, totalLength);
	// sendBuffer.rptr() = sendBuffer.sptr();
	return fieldIndex;
}
