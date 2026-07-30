#pragma once
#include <unistd.h>
#include "core.hpp"

template <usize bufferSize>
class Buffer {
public:
// 
	i32 fd;
	u32 index, size;
	u8 data[bufferSize - 3 * sizeof(u32)];

	isize read(usize bytes) {
		if (size + bytes > sizeof(data))
			return -1;	// ERROR: Buffer overflow

		isize bytesRead = ::read(fd, data + size, bytes);
		if (bytesRead < 0)
			return -2;
		size += (usize) bytesRead;	// TODO: what do we do on failures?
		return bytesRead;
	}

	isize write(usize bytes) {
		return write(fd, bytes);
	}

	isize write(i32 fdOverride, usize bytes) {
		usize bytesCapped = MIN(bytes, size - index);
		isize bytesWritten = ::write(fdOverride, data + index, bytesCapped);
	
		if (bytesWritten < 0)
			return bytesWritten;
		index += (u32) bytesWritten;
		if (size - index <= 32 && index < sizeof(data) - 32) {
			MEMMOVE_BUILTIN(data, data + index, 32);
			size -= index;
			index = 0;
		}
		return bytesWritten;
	}

	void clear() {
		fd = -1;
		index = 0;
		size = 0;
	}

	u32 find_line_end() {
		u32 lineEnd = UINT32_MAX;
		while (index < size - 1) {
			if (data[index] == '\r' && data[index + 1] == '\n') {
				lineEnd = index;
				index += 2;
				break;
			}
			else
				index++;
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

	// Should be impossible for dst buffer to not fit
	usize append(Buffer &src, usize length) {
		usize remainingSrc = src.size - src.index;	// How many bytes it has read
		usize remainingDst = sizeof(data) - size;	// How many bytes are free in the buffer
		usize appendLength = MIN3(length, remainingSrc, remainingDst);
	
		MEMCPY_BUILTIN(data + size, src.data + index, appendLength);
		src.index += appendLength;
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

	Buffer()
		: fd(-1), index(0), size(0)
		{
		}
};

// template <usize bufferSize>
// union u_buffer {
// 	struct s_buffer {
// 		Buffer<bufferSize> reader;
// 		Buffer<bufferSize> writer;
// 	};
// 	Buffer<bufferSize * 2> whole;
// };