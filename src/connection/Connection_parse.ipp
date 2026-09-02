#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) parse_line(usize lineLength) {
	if (lineLength < 2 || lineLength >= 8000) {
		status = lineLength < 2 ? Status::i400 : Status::i431;
		return -1;
	}
	isize fieldIndex;
	char* lineEnd = recvBuffer.rptr() + lineLength;
	Span field = sendBuffer.find_char(':');
	if (field.ptr == NULL)
		goto Error;
	fieldIndex = fn::match_field(field);
	if (!recvBuffer.skip_spaces())	// Reject empty values
		goto Error;
	switch (fieldIndex) {
		default:
			recvBuffer.readPos = (usize)(lineEnd - (char*)recvBuffer.data);
			break;

		case Field::CONNECTION:
			if (recvBuffer.strcasecmp("keep-alive"))	// not adding another bit just to check
				options |= Options::KEEP_ALIVE;			// if had been set already
			else if (recvBuffer.strcasecmp("close"))	// last setting counts
				options &= ~(u16)Options::KEEP_ALIVE;
			else
				goto Error;
		break;

		case Field::TRANSFER_ENCODING:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				goto Error; // ERROR: transfer method already set
			if (recvBuffer.strcasecmp("chunked") == false)
				goto Error; // ERROR: transfer encoding isnt chunked
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
		case Field::CONTENT_TYPE:
			while ((lineEnd[-1] == ' ' || lineEnd[-1] == '\t'))
				lineEnd--;
			req.contentTypeHeader.ptr = (char*) recvBuffer.rptr();
			req.contentTypeHeader.size = (usize) (lineEnd - recvBuffer.rptr());
			recvBuffer.readPos = (usize)(lineEnd - (char*)recvBuffer.data);
			break;

		case Field::HOST:
			if (options & Options::HOST)
				goto Error;	// ERROR: Multiple hosts
			while ((lineEnd[-1] == ' ' || lineEnd[-1] == '\t'))
				lineEnd--;
			req.host.ptr = (char*)recvBuffer.rptr();
			req.host.size = (usize)(lineEnd - recvBuffer.rptr());
			recvBuffer.readPos = (usize)(lineEnd - (char*)recvBuffer.data);
			options |= Options::HOST;
			break;
		case Field::COOKIES:
			while ((lineEnd[-1] == ' ' || lineEnd[-1] == '\t'))
				lineEnd--;
			req.cookies.ptr = (char*) recvBuffer.rptr();
			req.cookies.size = (usize) (lineEnd - recvBuffer.rptr());
			recvBuffer.readPos = (usize)(lineEnd - (char*)recvBuffer.data);
			break;
	}

	if (recvBuffer.skip_spaces())
		goto Error;
	recvBuffer.readPos = recvBuffer.scanPos;
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
(isize) parse_cgi_line(Buffer64 &dst) {
	const char* const lineEnd = (char*)sendBuffer.sptr() - 2;
	const usize totalLength = (usize)(lineEnd - (char*)sendBuffer.rptr());

	Span field = sendBuffer.find_char(':');
	if (field.ptr == NULL) {
		status = Status::i500;
		return -1;
	}

	const isize fieldIndex = fn::match_field(field);
	if (fieldIndex <= 0) {
		dst.append(field.ptr, totalLength);
		dst.append("\r\n");
		return fieldIndex;
	}

	if (fieldIndex == Field::STATUS) {
		sendBuffer.skip_spaces();
		status = sendBuffer.rptr();
		isize rvalue = status.is_valid() == true ? 0 : -1;
		if (rvalue == -1)
			status = Status::i500;	// CGI output an invalid status, should be server error
		return rvalue;
	}

	dst.append(field.ptr, totalLength);
	dst.append("\r\n");
	return fieldIndex;
}
