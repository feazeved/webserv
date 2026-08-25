#pragma once
#include "Buffer.hpp"

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer, 
// effectively performing compaction.

// Scanptr needs to be used to remember what was written or not

BUFFER_INL
(isize) dechunk(HTTP_Buffer& tmp, usize &chunkSize, usize &bodySize) {
	const u8 *const searchEnd = writePtr > data ? writePtr - 1 : data;

	while (readPtr < searchEnd) {
		if (chunkSize == SIZE_MAX) {
			usize lineLength = find_line_end();
			if (lineLength == SIZE_MAX)
				break;
			chunkSize = strtol16();
				// TODO: skip spaces here
			if (chunkSize == 0) {
				if (lineLength != 0)
					return -1;	// CLOSING ERROR: no header end after 0
				bodySize = 0;
				break;
			}
			if (chunkSize > bodySize)
				return -1;			// CLOSING ERROR: Body size was greater than maximum allowed or Wrong
			bodySize -= chunkSize;
		}
		else if (chunkSize > 0) {
			usize bytesAppended = tmp.append_buffer(*this, chunkSize);
			chunkSize -= bytesAppended;	// Guaranteed to be chunksize or less
		}
		else {
			if (!strcmp("\r\n"))
				return -1;
			chunkSize = SIZE_MAX;
		}
	}

	return tmp.writePtr - tmp.data;
}

// TODO: The buffer is only going to fill with data related to the chunks, decide if compaction is worth given current length
// Optimization opportunity here to have src copy directly to itself
// Otherwise just prepend the remainder to the end of what was read
BUFFER_INL
(isize) decode(int writeFd, usize &chunkSize, usize &bodySize) {
	Buffer<sizeof(*this)> tmpBuffer;

	if (dechunk(tmpBuffer, chunkSize, bodySize) < 0)
		return -1;

	isize bytesWritten = tmpBuffer.write(writeFd, ATOMIC_IOSIZE); // This is always going to be a server write
	if (writePtr > readPtr) { 
		isize bytesRemaining = writePtr - readPtr;
		tmpBuffer.append((usize) bytesRemaining);
	}

	if (tmpBuffer.writePtr > tmpBuffer.readPtr)
		copy(tmpBuffer);

	return bytesWritten;
}

BUFFER_INL
(isize) check_target(Span16 &path, Span16 &query) {
	u8 *const lineStart = readPtr;

	if (*readPtr != '/')
		return -1;
	query.index = 0;
	path.length = 0;
	path.index = 0;
	path.length = scanPtr - readPtr;
	while (readPtr < scanPtr) {
		if (g_asciiLut[*readPtr] > ASCII_RFC_SYMBOLS) {
			if (*readPtr != '?')
				return -1;
			path.length = readPtr - lineStart;
			readPtr++;
			query.index = readPtr - data;
			path.length = scanPtr - readPtr;
			break;
		}
		if (*readPtr == '%') {
			if (!(g_asciiLut[readPtr[1]] <= ASCII_HEX && g_asciiLut[readPtr[2]] <= ASCII_HEX))
				return -1;
			readPtr += 2;
		}
		readPtr++;
	}
	return 0;
}
