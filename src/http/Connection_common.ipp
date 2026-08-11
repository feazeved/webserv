#pragma once
#include "Connection.hpp"

namespace HTTP {

// Connection returns whether or not the FD should be removed from epoll
// 0: keep it, -1 remove it
CONNECTION_INL
(isize) dispatch(u32 events) {
	isize rvalue;

	if (state & State::READING_FROM_CLIENT) {
		rvalue = read_from_client(events);
		if (rvalue < 0)
			return error_path();
	}

	if (state & State::PARSING) {
		if (state & State::FIRST_LINE) {
			if (clientOutput.cursor.find_line_end() == 0)
				return 0;	// Need to read more
			if (request.parse_first_line(clientOutput.cursor, cfg) != 0)
				return error_path();
			state ^= State::FIRST_LINE;
		}
		rvalue = request.parse_header(clientOutput.cursor);
		if (rvalue == 0)
			return rvalue;
		else if (rvalue < 0)
			return error_path();
		if (configure() < 0)
			return error_path();
		return 0;
	}

	switch (request.mode) {
		case Mode::GET:		rvalue = get_method();	break;
		case Mode::POST:	rvalue = post_method();	break;
		case Mode::CGI:		rvalue = cgi_method();	break;
		case Mode::SSE:		rvalue = sse_method();	break;
		default:			break;
	}

	if (rvalue < 0)
		return error_path();

	if (state & State::WRITING_TO_CLIENT) {
		rvalue = write_to_client(events);
		if (rvalue < 0)
			return error_path();
	}

	return 0;
}

CONNECTION_INL
(isize) read_from_server() {
	isize bytesRead = clientInput.cursor.read(readFd, ATOMIC_IOSIZE);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
	}
	return bytesRead;
}

CONNECTION_INL
(isize) write_to_server() {
	isize bytesWritten;

	if (request.options & Options::CHUNKED_LENGTH)
		bytesWritten = decode();
	else {
		bytesWritten = clientOutput.cursor.write(writeFd, request.bodySize);
		if (bytesWritten > 0)
			request.bodySize -= (usize) bytesWritten;
	}

	if (request.bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
	}
	return bytesWritten;
}

// Common to all
// TODO: epoll event checks to see if valid
CONNECTION_INL
(isize) write_to_client(u32 events) {
	isize bytesWritten = clientInput.cursor.write(clientFd, ATOMIC_IOSIZE);
	if (bytesWritten < 0)
		return bytesWritten;	// TODO: tmp error path
	return bytesWritten;
}

// Common to POST and CGI
// TODO: epoll event checks to see if valid
CONNECTION_INL
(isize) read_from_client(u32 events) {
	if (clientOutput.cursor.readPtr < clientOutput.cursor.writePtr)	// Still have things to process
		return 0;
	isize bytesRead = clientOutput.cursor.read(clientFd, ATOMIC_IOSIZE);
	if (bytesRead < 0)
		return bytesRead;
	return bytesRead;
}

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer, 
// effectively performing compaction.
CONNECTION_INL
(isize) dechunk(Cursor& src, Cursor& dst) {
	const u8 *const searchEnd = src.writePtr > src.memStart ? src.writePtr - 1 : src.memStart;

	while (src.readPtr < searchEnd) {
		if (request.chunkSize == SIZE_MAX) {
			isize rvalue = src.find_line_end();
			if (rvalue == 0)
				break;
			request.chunkSize = src.strtol16();
			if (request.chunkSize == 0) {
				if (rvalue != 2)
					return -1;	// CLOSING ERROR: no header end after 0
				request.bodySize = 0;
				break;
			}
			if (request.chunkSize > request.bodySize)
				return -1;			// CLOSING ERROR: Body size was greater than maximum allowed or Wrong
			request.bodySize -= request.chunkSize;
		}
		else if (request.chunkSize > 0) {
			usize bytesAppended = dst.append(src, request.chunkSize);
			request.chunkSize -= bytesAppended;	// Guaranteed to be chunksize or less
		}
		else {
			if (!src.strcmp("\r\n"))
				return -1;
			request.chunkSize = SIZE_MAX;
		}
	}

	return dst.writePtr - dst.memStart;
}

// TODO: The buffer is only going to fill with data related to the chunks, decide if compaction is worth given current length
// Optimization opportunity here to have src copy directly to itself
// Otherwise just prepend the remainder to the end of what was read
CONNECTION_INL
(isize) decode() {
	Buffer<sizeof(clientInput)> tmpBuffer;
	Cursor &src = clientOutput.cursor;
	Cursor &tmp = tmpBuffer.cursor;

	if (dechunk(src, tmp) < 0)
		return -1;

	isize bytesWritten = tmp.write(writeFd, ATOMIC_IOSIZE); // This is always going to be a server write
	if (src.writePtr > src.readPtr) { 
		isize bytesRemaining = src.writePtr - src.readPtr;
		tmp.append(src, (usize)bytesRemaining);
	}

	if (tmp.writePtr > tmp.readPtr)
		src.copy(tmp);

	return bytesWritten;
}
}
