#pragma once
#include <unistd.h>
#include "core.hpp"

// template <usize bufferSize>
// union u_buffer {
// 	struct s_buffer {
// 		Buffer<bufferSize> reader;
// 		Buffer<bufferSize> writer;
// 	};
// 	Buffer<bufferSize * 2> whole;
// };

template <usize bufferSize>
class Buffer {
public:
// 
	i32 fd;
	u8 data[bufferSize - 3 * sizeof(u32)];
	u32 index, size;

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
		if (index == size)
			return 0;	// Nothing to write, should be an error if the payload isnt 0

		bytes = MIN(bytes, size - index);
		isize bytesWritten = ::write(fd, data + index, bytes);
		if (bytesWritten < 0)
			return -1;
		index += (u32) bytesWritten;
		return bytesWritten;
	}

	isize write(i32 fdOverride, usize bytes) {
		if (index == size)
			return 0;	// Nothing to write, should be an error if the payload isnt 0

		bytes = MIN(bytes, size - index);
		isize bytesWritten = ::write(fdOverride, data + index, bytes);
		if (bytesWritten < 0)
			return -1;
		index += (u32) bytesWritten;
		return bytesWritten;
	}

	void clear() {
		fd = -1;
		index = 0;
		size = 0;
	}

	Buffer()
		: fd(-1), index(0), size(0)
		{
		}
};

template <usize bufferSize>
class IOBuffer {
public:
	Buffer<bufferSize> reader, writer;

	isize read(usize bytes) {
		return reader.read(bytes);
	}

	isize read(usize bytes, u32 events) {
		// Epoll check
		return reader.read(bytes);
	}

	isize write(usize bytes) {
		return writer.write(bytes);
	}
	isize write(usize bytes, u32 events) {
		// Epoll check
		return writer.write(bytes);
	}

	void clear() {
		// TODO: close fd
		if (reader.fd == writer.fd) {
			close(reader.fd);
		}
		else {
			close(reader.fd);
			close(writer.fd);
		}

		reader.clear();
		writer.clear();
	}
};
