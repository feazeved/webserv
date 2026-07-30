#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "HTTP.hpp"

// TODO: add static assertions to sizeof(buffer) being power of two
// TODO: minimize compaction by assessing distance from newline to writeoffset and memcpying
	// So basically check if the tail is below 32 bytes and memcpy to beginning
// TODO: only compact after execve, and only when necessary to avoid CoWs

// There are two fds: the server will read from the clientFd to the input buffer,
// and the server will read from clientFD to the output buffer
template <usize bufferSize>
class Buffer {
public:
	u8 data[bufferSize - 2 * sizeof(u32)];
	u32 cursor, size;

	// Methods
	// 0) No reads, 1) Read, -1) Failed Reading, -2) Line is too big
	i8 read(i32 fd, usize bytes, u32 events) {
		if ((events & EPOLLIN) == 0)
			return -1;		// ERROR: Attempted to read but epoll was not set or ready

		if (cursor + 1 < size)
			return 0;
		if (size + bytes > sizeof(data))
			return -1;	// ERROR: Line is too long

		isize bytesRead = ::read(fd, data + size, bytes);
		if (bytesRead < 0)
			return -2;	// TODO: Need to distinguish between first read fail and other read fails

		size += (usize) bytesRead;	// TODO: what do we do on failures?
		return 1;
	}

	i8 write(i32 fd, usize bytes, u32 events) {
		if ((events & EPOLLOUT) == 0)
			return -1;		// ERROR: Attempted to write but epoll was not set or ready

		if (cursor + 1 < size)
			return 0;	// Nothing to write, should be an error if the payload isnt 0

		bytes = MIN(bytes, size - cursor);
		isize bytesWritten = ::write(fd, data + cursor, bytes);	// TODO: The write here reads from a different FD, and from a different buffer too
		if (bytesWritten < 0)
			return -1;
		return 1;
	}

	u32 find_line_end() {
		u32 lineEnd = UINT32_MAX;
		while (cursor < size - 1) {
			if (data[cursor] == '\r' && data[cursor + 1] == '\n') {
				lineEnd = cursor;
				cursor += 2;
				break;
			}
			else
				cursor++;
		}
		return lineEnd;
	}

	template <usize N>
	void append(const char (&string)[N]) {
		MEMCPY_INLINE(data + size, string, N - 1);
		size += N - 1;
	}

	void append(const u8 *ptr, usize length) {
		MEMCPY_BUILTIN(data + size, ptr, length);
		size += length;
	}

	// Should actually be called write, because this needs to release 
	usize append(Buffer &src, usize length) {
		usize remainingSrc = src.size - src.cursor;	// How many bytes it has read
		usize remainingDst = sizeof(data) - size;	// How many bytes are free in the buffer
		usize appendLength = MIN3(length, remainingSrc, remainingDst);
	
		MEMCPY_BUILTIN(data + size, src.data + cursor, appendLength);
		src.cursor += appendLength;
		size += appendLength;
		return appendLength;
	}

	void append(usize number, bool isHex) {
		static const char digits[16] = {
			'0', '1', '2', '3', '4', '5', '6', '7',
			'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
		char	buffer[24];
		usize	length;
		usize	i = sizeof(buffer);

		buffer[--i] = '\n';
		buffer[--i] = '\r';
		if (isHex)
		{
			do
			{
				buffer[--i] = digits[(number % 16)];
				number /= 16;
			}	while (number != 0);
		}
		else
		{
			do
			{
				buffer[--i] = digits[(number % 10)];
				number /= 10;
			}	while (number != 0);			
		}
		length = sizeof(buffer) - i;
		MEMCPY_BUILTIN(data + size, buffer + i, length);
		size += length;
	}

	void clear() {
		cursor = 0;
		size = 0;
	}

	usize capacity() {
		return sizeof(data);
	}
};

// #define BUFFER_INLINE_APPEND(buf, string)                      \
// 	do                                                            \
// 	{                                                             \
// 		const usize length = sizeof(string) - 1;                  \
// 		MEMCPY_INLINE(                                            \
// 			(buf).data + (buf).writeOffset,                       \
// 			(string),                                             \
// 			length);                                              \
// 		(buf).writeOffset += length;                              \
// 	} while (0)