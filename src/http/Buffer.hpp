#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "HTTP.hpp"

// TODO: add static assertions to sizeof(buffer) being power of two

// There are two fds: the server will read from the clientFd to the input buffer,
// and the server will read from clientFD to the output buffer
template <usize bufferSize>
class Buffer {
public:
	u8 data[bufferSize - 2 * sizeof(u32)];
	u32 readOffset, writeOffset;

	// Methods
	// 0) No reads, 1) Read, -1) Failed Reading, -2) Line is too big
	i8 read(i32 fd, usize bytes, u32 events) {
		if ((events & EPOLLIN) == 0)
			return -1;		// ERROR: Attempted to read but epoll was not set or ready

		if (readOffset + 1 < writeOffset)
			return 0;
		if (writeOffset + bytes > sizeof(data))
			return -1;	// ERROR: Line is too long

		isize bytesRead = ::read(fd, data + writeOffset, bytes);
		if (bytesRead < 0)
			return -2;	// TODO: Need to distinguish between first read fail and other read fails

		writeOffset += (usize) bytesRead;	// TODO: what do we do on failures?
		return 1;
	}

	i8 readFile(i8 *path, usize reqBytes);
	i8 writeFile(i8 *path, usize reqBytes);

	i8 write(i32 fd, usize bytes, u32 events) {
		if ((events & EPOLLOUT) == 0)
			return -1;		// ERROR: Attempted to write but epoll was not set or ready

		if (readOffset + 1 < writeOffset)
			return 0;	// Nothing to write, should be an error if the payload isnt 0

		bytes = MIN(bytes, writeOffset - readOffset);
		isize bytesWritten = ::write(fd, data + readOffset, bytes);	// TODO: The write here reads from a different FD, and from a different buffer too
		if (bytesWritten < 0)
			return -1;
		return 1;
	}

	// TODO: Passing state as reference is less clean because state is being mutated from a method outside of Request
	u32 find_line_end(bool &header_done) {

		u32 lineEnd = UINT32_MAX;
		while (readOffset < writeOffset - 1) {
			if (data[readOffset] == '\r' && data[readOffset + 1] == '\n')
			{
				lineEnd = readOffset;
				readOffset += 2;
				if (readOffset < writeOffset - 1 && data[readOffset] == '\r' && data[readOffset + 1] == '\n') {
					readOffset += 2;
					header_done = true;
					break;
				}
			}
			else
				readOffset++;
		}
		return lineEnd;
	}

	template <usize N>
	void append(const char (&string)[N]) {
		MEMCPY_INLINE(data + writeOffset, string, N - 1);
		writeOffset += N - 1;
	}

	void append(const u8 *ptr, u32 length) {
		MEMCPY_BUILTIN(data + writeOffset, ptr, length);
		writeOffset += length;
	}

	void append(usize number, bool isHex) {
		char	buffer[24];
		usize	base = isHex ? 16 : 10;
		usize	length;
		usize	i = sizeof(buffer);

		buffer[--i] = '\n';
		buffer[--i] = '\r';
		do
		{
			buffer[--i] = (number % base) + '0';
			number /= base;
		}	while (number != 0);
		length = sizeof(buffer) - i;
		MEMCPY_BUILTIN(data + writeOffset, buffer + i, length);
		writeOffset += length;
	}

	void clear() {
		readOffset = 0;
		writeOffset = 0;
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