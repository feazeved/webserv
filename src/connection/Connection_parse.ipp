#pragma once
#include "Connection.hpp"

CONNECTION_INL
(Status::Code) parse_line(Span line) {
	if (line.size < 2 || line.size >= 8000)
		return line.size < 2 ? Status::i400 : Status::i431;

	const usize readEnd = recvBuffer.readPos + line.size;
	Span field = sendBuffer.find_char(':');
	if (field.ptr == NULL)
		return Status::i400;

	isize fieldIndex = fn::match_field(field);
	Span value = recvBuffer.get_field_value(readEnd);
	if (value.ptr == NULL)
		return Status::i400;	// Rejects empty values
	switch (fieldIndex) {
		default:
			recvBuffer.readPos = readEnd;
			break;
// ============================================================================
		case Field::TRANSFER_ENCODING:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				return Status::i400; // ERROR: transfer method already set
			if (value.strcasecmp("chunked") == false)
				return Status::i400; // ERROR: transfer encoding isnt chunked
			options |= Options::CHUNKED_LENGTH;
			break;
// ============================================================================
		case Field::CONTENT_LENGTH:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				return Status::i400; // ERROR: bad request, transfer method had already been set
			bodySize = fn::strtol10(value.ptr);
			if (bodySize == SIZE_MAX)
				return Status::i400;
			if (bodySize > cfg->maxBodySize)
				return Status::i413;
			options |= Options::FIXED_LENGTH;
			break;
// ============================================================================
		case Field::CONTENT_TYPE:
			req.contentTypeHeader = value;
			break;
// ============================================================================
		case Field::HOST:
			if (options & Options::HOST)
				return Status::i400;	// ERROR: Multiple hosts
			req.host = value;
			options |= Options::HOST;
			break;
// ============================================================================
		case Field::CONNECTION:
			if (value.strcasecmp("keep-alive"))	// not adding another bit just to check
				options |= Options::KEEP_ALIVE;			// if had been set already
			else if (value.strcasecmp("close"))	// last setting counts
				options &= ~(u16)Options::KEEP_ALIVE;
			else
				return Status::i400;
		break;
// ============================================================================
		case Field::COOKIES:
			req.cookies = value;
			break;
	}
	return Status::unset;
}

/*	CGI output is server-controlled, so this path performs only the inexpensive
	structural checks needed before forwarding the line
*/
CONNECTION_INL
(Status::Code) parse_cgi_line(Buffer64 &dst) {
	const char* const lineEnd = (char*)sendBuffer.sptr() - 2;
	const usize totalLength = (usize)(lineEnd - (char*)sendBuffer.rptr());

	Span field = sendBuffer.find_char(':');
	if (field.ptr == NULL)
		return Status::i500;

	const isize fieldIndex = fn::match_field(field);
	if (fieldIndex <= 0) {
		dst.append(field.ptr, totalLength);
		dst.append("\r\n");
		return Status::ok;
	}

	if (fieldIndex == Field::STATUS) {
		sendBuffer.skip_spaces();
		Status::Code code = Status::s_str_to_code(sendBuffer.rptr());	// TODO: change the check to be if OK not if error
		return (code == Status::ixxx) ? Status::i500 : Status::ok;	// CGI output an invalid status, should be server error
	}

	dst.append(field.ptr, totalLength);
	dst.append("\r\n");
	return Status::ok;
}
