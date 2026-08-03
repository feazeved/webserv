#pragma once
#include <unistd.h>
#include "core.hpp"
#include "Request_helpers.inl"

template <usize bufferSize>
class Buffer {
private:

public:

	union {
		u8 rawData[bufferSize];
		struct {
			u8 data[bufferSize - sizeof(usize) * 8];	// Last cache line is reserved for unbounded memory loads
			usize reserved[4];
			usize index, size, start, end;
		};
	};

	isize read(i32 fd, usize bytes) {
		if (size + bytes > sizeof(data))
			return -1;	// ERROR: Buffer overflow

		isize bytesRead = ::read(fd, data + size, bytes);
		if (bytesRead < 0)
			return -2;
		size += (usize) bytesRead;	// TODO: what do we do on failures?
		return bytesRead;
	}

	isize write(i32 fd, usize bytes) {
		usize bytesCapped = MIN(bytes, size - index);
		isize bytesWritten = ::write(fd, data + index, bytesCapped);
	
		if (bytesWritten < 0)
			return bytesWritten;
		index += (u32) bytesWritten;
		if (size - index <= 32) {	// Check if this is needed
			MEMMOVE(data, data + index, 32);
			size -= index;
			index = 0;
		}
		return bytesWritten;
	}

	void reset() {
		index = 0;
		size = 0;
	}

	usize find_line_end() {
		usize lineEnd = SIZE_MAX;
		const u32 maxLength = size == 0 ? 0 : size - 1;

		while (index < maxLength) {
			if (MEMCMP(data + index, "\r\n", 2) == 0) {
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
		MEMCPY(data + size, ptr, length);
		size += length;
	}

	// Should be impossible for dst buffer to not fit
	// TODO: Might remove MIN3 and have it overflow to guarantee behavior
	usize append(Buffer &src, usize length) {
		usize remainingSrc = src.size - src.index;	// How many bytes it has read
		usize remainingDst = sizeof(data) - size;	// How many bytes are free in the buffer
		usize appendLength = MIN3(length, remainingSrc, remainingDst);
	
		MEMCPY(data + size, src.data + index, appendLength);
		src.index += appendLength;
		size += appendLength;
		return appendLength;
	}

	// TODO: No length checks
	// TODO: separate functions
	void append(usize number) {
		char buffer[48];
		char *mid = buffer + 24;
		usize digitLength = s_itoa10(number, mid);
		char *start = buffer + 24 - digitLength;

		*mid++ = '\r';
		*mid = '\n';

		MEMCPY_INLINE(data + size, start, 24);
		size += digitLength;
	}

	Buffer()
		: index(0), size(0)
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