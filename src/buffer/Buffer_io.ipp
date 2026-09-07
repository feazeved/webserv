#pragma once
#include "Buffer.hpp"

BUFFER_INL
(usize) compact() {
	const usize bytesUsed = writePos - readPos;
	const usize scanOffset = scanPos - readPos;

	MEMMOVE(data, data + readPos, bytesUsed);
	readPos = 0;
	scanPos = scanOffset;
	writePos = bytesUsed;
	return sizeof(data) - writePos;
}

BUFFER_INL
(isize) read_compact(int fd, usize bytes) {
	usize bytesFree = sizeof(data) - writePos;

	if (bytesFree < bytes) {
		bytesFree = compact();
		if (bytesFree == 0)
			return -2;
	}

	const usize bytesCapped = MIN(bytesFree, bytes);
	isize bytesRead = ::read(fd, data + writePos, bytesCapped);
	if (bytesRead < 0)
		return -1;
	writePos += (usize) bytesRead;
	return bytesRead;
}

BUFFER_INL
(isize) read(int fd, usize bytes) {
	usize bytesFree = sizeof(data) - writePos;

	if (bytesFree < bytes)
		return -1;
	const usize bytesCapped = MIN(bytesFree, bytes);
	isize bytesRead = ::read(fd, data + writePos, bytesCapped);
	if (bytesRead > 0)
		writePos += (usize) bytesRead;
	return bytesRead;
}

BUFFER_INL
(isize) write(int fd, usize bytes) {
	usize bytesCapped = MIN3(ATOMIC_IOSIZE, bytes, writePos - readPos);
	isize bytesWritten = ::write(fd, data + readPos, bytesCapped);

	if (bytesWritten > 0) {
		readPos += (usize) bytesWritten;
		scanPos = (scanPos >= readPos) ? scanPos : readPos;
	}
	return bytesWritten;
}
