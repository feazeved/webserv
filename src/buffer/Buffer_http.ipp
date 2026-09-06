#pragma once
#include "Buffer.hpp"

/*
	chunkSize == 0 means we're reading the chunk header
	bodySize here refers to the total specified in the config

*/

BUFFER_INL
(Status::Code) dechunk(Buffer& tmp, usize &chunkSize, usize &bodySize) {
	while (writePos - readPos > 5) {
// ==== Reading chunk header ==================================================
		if (chunkSize == 0) {
			if (LITCMP(data + readPos, "0\r\n\r\n") == 0) {
				readPos += 5;
				scanPos = readPos;
				return Status::ok;
			}
			if (find_line_end().ptr == NULL)
				return Status::unset;
			chunkSize = fn::strtol16((char*)data + readPos);
			if (chunkSize == 0)
				return Status::i400;
			if (chunkSize == SIZE_MAX)
				return Status::i400;
			if (chunkSize > bodySize)
				return Status::i413;
			bodySize -= chunkSize;
			readPos = scanPos;		// Everything after the digit is ignored
		}
// ==== Reading chunk body ====================================================
		else {
			usize appendLength = MIN(chunkSize, writePos - readPos - 2);
			tmp.append(data + readPos, appendLength);
			readPos += appendLength;
			scanPos = readPos;
			chunkSize -= appendLength;	// Guaranteed to be chunksize or less
			if (chunkSize == 0) {
				if (!LITCMP(data + readPos, "\r\n"))
					return Status::i400;
				readPos += 2;
				scanPos = readPos;
			}
		}
	}
	return Status::unset;
}
