#pragma once
#include "Buffer.hpp"

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer,
// effectively performing compaction.

// Scanptr needs to be used to remember what was written or not

BUFFER_INL
(isize) dechunk(HTTP_Buffer& tmp, usize &chunkSize, usize &bodySize) {
	const usize searchEnd = writePos >= 4 ? writePos - 3 : 0;

	while (readPos < searchEnd) {
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

	return 0;
}

// TODO: The buffer is only going to fill with data related to the chunks, decide if compaction is worth given current length
// Optimization opportunity here to have src copy directly to itself
// Otherwise just prepend the remainder to the end of what was read
BUFFER_INL
(isize) decode(int writeFd, usize &chunkSize, usize &bodySize) {
	if (readPos < scanPos)
		return write(writeFd, MIN(scanPos - readPos, (usize)ATOMIC_IOSIZE));

	Buffer<sizeof(*this)> tmpBuffer;

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

BUFFER_INL
(isize) check_target(Span &path, Span &query) {
	const usize lineStart = readPos;

	if (data[readPos] != '/')
		return -1;
	path.ptr = (char*)data + readPos;
	path.length = scanPos - readPos;
	query.ptr = (char*)data + scanPos;
	query.length = 0;
	while (readPos < scanPos) {
		if (g_asciiLut[data[readPos]] > ASCII_RFC_SYMBOLS) {
			if (data[readPos] != '?')
				return -1;
			path.length = readPos - lineStart;
			readPos++;
			query.ptr = (char*)data + readPos;
			query.length = scanPos - readPos;
			break;
		}
		if (data[readPos] == '%') {
			if (!(g_asciiLut[data[readPos + 1]] <= ASCII_HEX && g_asciiLut[data[readPos + 2]] <= ASCII_HEX))
				return -1;
			readPos += 2;
		}
		readPos++;
	}
	return 0;
}
