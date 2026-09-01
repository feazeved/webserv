#pragma once
#include "Buffer.hpp"

// This function dechunks from a source buffer to a stack buffer, then writes from this stack buffer
// Any bytes that weren't consumed by the write are copied back to the start of the source buffer,
// effectively performing compaction.

// Scanptr needs to be used to remember what was written or not
/*
	Assuming chunk extensions and trailers are intentionally unsupported, there were three concrete state-machine bugs.

	For a valid ending:

	0\r\n\r\n

	find_line_end() returns 1 for the line "0", so this rejects every valid terminal chunk:

	if (lineLength != 0)
		return -1;

	bodySize became 0 immediately after 0\r\n, before consuming the required final \r\n.
	Consuming a chunk’s trailing \r\n advanced readPos but not scanPos, so find_line_end() could rediscover already-consumed bytes.
	Reserving three bytes with searchEnd could stall when exactly the required two-byte delimiter was available.
*/

BUFFER_INL
(isize) dechunk(Buffer& tmp, usize &chunkSize, usize &bodySize) {
	const usize searchEnd = writePos >= 4 ? writePos - 3 : 0;

	while (readPos < searchEnd) {
		if (chunkSize == SIZE_MAX) {
			usize lineLength = find_line_end();
			if (lineLength == SIZE_MAX)
				break;
			chunkSize = strtol16();
			if (chunkSize > bodySize || !strcmp("\r\n"))
				return -1;			// Body size was greater than maximum allowed or Wrong
			if (chunkSize == 0) {
				if (lineLength != 0)
					return -1;	// CLOSING ERROR: no header end after 0
				bodySize = 0;
				break;
			}
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

/*
Three real issues:

It truncates targets without a query:
data[readPos - 1] = 0;

For /file.txt, readPos == targetEnd, so this overwrites the final t. The caller had already placed '\0' at targetEnd.

It stops scanning after ?, so the query is never checked for invalid control characters, whitespace, or malformed percent sequences.
It accepts raw .. path segments. Since the target suffix is later appended to root or upload_store, /public/../secret can escape the configured directory.

*/

BUFFER_INL
(isize) check_target(Span &path, Span &query, usize targetEnd) {
	const usize lineStart = readPos;

	if (data[readPos] != '/')
		return -1;
	path.ptr = (char*)data + readPos;
	path.size = targetEnd - readPos;
	query.ptr = (char*)data + targetEnd;
	query.size = 0;
	while (readPos < targetEnd) {
		if (g_asciiLut[data[readPos]] > ASCII_RFC_SYMBOLS) {
			if (data[readPos] != '?')
				return -1;
			path.size = readPos - lineStart;
			readPos++;
			query.ptr = (char*)data + readPos;
			query.size = targetEnd - readPos;
			break;
		}
		if (data[readPos] == '%') {
			if (!(g_asciiLut[data[readPos + 1]] <= ASCII_HEX && g_asciiLut[data[readPos + 2]] <= ASCII_HEX))
				return -1;
			readPos += 2;
		}
		readPos++;
	}
	data[readPos - 1] = 0;	// Null terminates target
	return 0;
}
