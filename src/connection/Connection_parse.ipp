#pragma once
#include "Connection.hpp"

CONNECTION_INL
(isize) parse(Epoll &epoll) {
	while (true) {
		Span line;
		while ((line = recvBuffer.find_line_end()) != NULL) {
			if (line.size == 0) {
				recvBuffer.readPos = recvBuffer.scanPos;
				return setup(epoll);
			}
			Status::Code code = parse_line(line);
			if (code != Status::unset)
				return flush_setup_close(epoll, code);
		}

		const isize result = read_from_client(epoll);
		if (result <= 0)
			return result;
	}
}

CONNECTION_INL
(Status::Code) parse_line(Span line) {
	if (line.size < 2 || line.size >= 8000)
		return line.size < 2 ? Status::i400 : Status::i431;

	const usize readEnd = recvBuffer.readPos + line.size;
	Span field = recvBuffer.find_char(':');
	if (field.ptr == NULL || field.size == 0)
		return Status::i400;

	isize fieldIndex = fn::match_field(field);
	Span value = recvBuffer.get_field_value(readEnd);
	if (value.ptr == NULL)
		return Status::i400;	// Rejects empty values
	switch (fieldIndex) {
		default: break;
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
			bodySize = fn::strtol10(value.ptr, value.size, value.size);
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
