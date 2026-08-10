#pragma once
#include "Connection.hpp"

namespace HTTP {

CONNECTION_INL
(isize) read_from_server(usize bytes) {
	isize bytesRead = clientInput.read(readFd, bytes);
	if (bytesRead == 0) {
		close(readFd);
		readFd = -1;
	}
	return bytesRead;
}

CONNECTION_INL
(isize) write_to_server(usize bytes) {
	isize bytesWritten;

	if (request.type & Attributes::CHUNKED)
		bytesWritten = dechunk(bytes, clientOutput);
	else {
		bytesWritten = clientOutput.write(writeFd, request.bodySize);
		if (bytesWritten > 0)
			request.bodySize -= bytesWritten;
	}

	if (request.bodySize == 0) {	// Must guarantee that bodySize is 0
		close(writeFd);
		writeFd = -1;	// Finished reading
	}
	return bytesWritten;
}

// Common to all
CONNECTION_INL
(isize) write_to_client(usize bytes, u32 events) {
	// TODO: epoll event checks to see if valid
	isize bytesWritten = clientInput.write(clientFd, bytes);
	if (bytesWritten < 0)
		return bytesWritten;	// TODO: tmp error path
	return bytesWritten;
}

// Common to POST and CGI
CONNECTION_INL
(isize) read_from_client(usize bytes, u32 events) {
	// TODO: epoll event checks to see if valid
	if (clientOutput.index < clientOutput.size)	// Still have things to process
		return 0;
	isize bytesRead = clientOutput.read(clientFd, bytes);
	if (bytesRead < 0)
		return bytesRead;
	return bytesRead;
}

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer, 
// effectively performing compaction.
CONNECTION_INL
(isize) dechunk(usize bytes, Buffer<bufferSize>& src) {
	Buffer<bufferSize> tmpBuffer;
	const usize maxLength = src.size != 0 ? src.size - 1 : 0;

	while (src.index < maxLength) {
		if (request.chunkSize == SIZE_MAX) {
			isize rvalue = src.find_line_end();
			if (rvalue == 0)
				break;
			request.chunkSize = s_strtol16(src.data + src.start);
			if (request.chunkSize == 0) {
				if (rvalue != 2)
					return -1;
				request.bodySize = 0;
				break;
			}
			if (request.chunkSize > request.bodySize)
				return -1;			// CLOSING ERROR: Body size was greater than maximum allowed or Wrong
			request.bodySize -= request.chunkSize;
		}
		else if (request.chunkSize > 0) {
			usize bytesAppended = tmpBuffer.append(src, request.chunkSize);
			request.chunkSize -= bytesAppended;	// Guaranteed to be chunksize or less
		}
		else {
			if (MEMCMP(src.data + src.index, "\r\n", 2) != 0)
				return -1;
			src.index += 2;
			request.chunkSize = SIZE_MAX;
		}
	}

	if (tmpBuffer.size == 0)
		return 0;				// CHECK: Nothing was appended

	isize bytesWritten = tmpBuffer.write(writeFd, bytes);	// Writes may be reattempted, so regardless it should compact

	// TODO: The buffer is only going to fill with data related to the chunks, decide if compaction is worth given current length
	// Optimization opportunity here to have src copy directly to itself
	// Otherwise just prepend the remainder to the end of what was read
	if (src.size > src.index) { 
		isize bytesRemaining = src.size - src.index;
		tmpBuffer.append(src, (usize)bytesRemaining);
	}

	if (tmpBuffer.size > tmpBuffer.index)
		src.copy(tmpBuffer);

	return bytesWritten;
}
}
