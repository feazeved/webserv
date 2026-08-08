#pragma once
#include <unistd.h>
#include "core.hpp"
#include "Connection_helpers.ipp"

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

bool is_full() {
	return size == sizeof(data);
}

isize find_line_end() {
	start = end != SIZE_MAX ? end : start;	// Previous call found a match
	end = SIZE_MAX;
	const usize maxLength = size == 0 ? 0 : size - 3;

	while (index < maxLength) {
		if (MEMCMP(data + index, "\r\n", 2) == 0) {
			index += 2;
			end = index;
			if (MEMCMP(data + index, "\r\n", 2) == 0) {
				index += 2;
				return 2; // Found header end
			}
			return 1; // Found line end
		}
		else
			index++;
	}
	return 0;
}

// Does not update start and end (not for line parsing)
bool find_header_end() {
	const usize maxLength = size == 0 ? 0 : size - 3;

	while (index < maxLength) {
		if (MEMCMP(data + index, "\r\n\r\n", 4) == 0) {
			index += 4;
			return true; // Found header end
		}
		else
			index++;
	}
	return false;
}

bool prepend(const u8 *ptr, usize length) {
	if (length > index)
		return false;
	index -= length;
	MEMCPY(data + index, ptr, length);
}

bool insert(const u8 *ptr, usize length, usize insertIndex) {
	MEMCPY(data + insertIndex, ptr, length);
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

void appendInline(const u8 *ptr, usize length) {
	MEMCPY_INLINE(data + size, ptr, length);
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

void copy(const Buffer& other) {
	size = other.size - other.index;
	index = 0;
	MEMCPY(data, other.data + other.index, size);
}

Buffer()
	: index(0), size(0), start(0), end(SIZE_MAX)
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
