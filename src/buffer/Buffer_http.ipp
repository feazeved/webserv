#pragma once
#include "Buffer.hpp"

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer,
// effectively performing compaction.

BUFFER_INL
(isize) dechunk(Buffer& tmp, usize &chunkSize, usize &bodySize) {
	const usize searchEnd = writePos >= 2 ? writePos - 1 : 0;

	while (readPos < searchEnd) {
		if (chunkSize == SIZE_MAX) { // Dechunking header
			Span line = find_line_end();
			if (line.ptr == NULL)
				break;
			chunkSize = fn::strtol16((char*)data + readPos);
			readPos = scanPos;			// Everything after the digit is ignored
			if (chunkSize > bodySize)
				return -1;				// Body size was greater than maximum allowed or Wrong
			if (chunkSize == 0) {
				if (line.size != 1)
					return -1;	// CLOSING ERROR: no header end after 0
				bodySize = 0;
				break;
			}
			bodySize -= chunkSize;
		}
		else if (chunkSize > 0) { // Dechunking body
			usize bytesAppended = tmp.append_buffer(*this, chunkSize);
			chunkSize -= bytesAppended;	// Guaranteed to be chunksize or less
		}
		else {
			if (!strcmp("\r\n"))
				return -1;
			chunkSize = SIZE_MAX;
		}
	}
	return 0;
}

// TODO: The buffer is only going to fill with data related to the chunks, decide if compaction is worth given current length
// Optimization opportunity here to have src copy directly to itself
// Otherwise just prepend the remainder to the end of what was read
BUFFER_INL
(isize) decode(int writeFd, usize &chunkSize, usize &bodySize) {
	if (readPos < scanPos)
		return write(writeFd, MIN(scanPos - readPos, (usize)ATOMIC_IOSIZE));

	Buffer tmpBuffer = {};

	if (dechunk(tmpBuffer, chunkSize, bodySize) < 0)
		return -1;
	if (tmpBuffer.size() == 0) {
		scanPos = readPos;
		compact();
		return 0;
	}

	isize bytesWritten = tmpBuffer.write(writeFd, ATOMIC_IOSIZE); // This is always going to be a server write
	const usize decodedRemaining = tmpBuffer.size();
	const usize rawRemaining = size();
	if (rawRemaining > 0)
		tmpBuffer.append((const char*)data + readPos, rawRemaining);
	bufcpy(tmpBuffer);
	scanPos = decodedRemaining;
	return bytesWritten;
}
