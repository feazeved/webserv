#pragma once
#include "Buffer.hpp"

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer,
// effectively performing compaction.

BUFFER_INL
(isize) dechunk(Buffer& tmp, usize &chunkSize, usize &bodySize) {
	while (readPos < writePos) {
		if (chunkSize == SIZE_MAX) {
			if (writePos - readPos < 5)
				break;
			Span line = find_line_end();
			if (line.ptr == NULL)
				break;
			chunkSize = fn::strtol16((char*)data + readPos);
			if (chunkSize > bodySize)
				return -1;				// Body size was greater than maximum allowed or Wrong
			if (chunkSize == 0) {
				if (line.size != 1 || MEMCMP(data + readPos + 1, "\r\n\r\n", 4) != 0)
					return -1;
				readPos += 5;
				scanPos = readPos;
				return 1;
			}
			bodySize -= chunkSize;
			readPos = scanPos;			// Everything after the digit is ignored
		}
		else if (chunkSize > 0) { // Dechunking body
			usize bytesAppended = tmp.append_buffer(*this, chunkSize);
			chunkSize -= bytesAppended;	// Guaranteed to be chunksize or less
		}
		else {
			if (writePos - readPos < 2)
				break;
			if (!strcmp("\r\n"))
				return -1;
			scanPos = readPos;
			chunkSize = SIZE_MAX;
		}
	}
	return 0;
}
