#pragma once
#include <unistd.h>

#include "core.hpp"


class Cursor {
public:
	u8 *ptr;
    i32 fd;
    u32 index;
    u32 size;

	void clear() {
		fd = -1;
		index = 0;
		size = 0;
	}

	isize write();
    Cursor()
        : fd(-1), index(0), size(0)
    {
    }
};

template <usize BufferSize>
class IOBuffer {
public:
    u8 data[BufferSize - 2 * sizeof(Cursor)];
	Cursor reader, writer;

	// Methods
	// 0) No reads, 1) Read, -1) Failed Reading, -2) Line is too big
	isize read(usize bytes) {
		if (reader.size + bytes > sizeof(data) / 2)
			return -1;	// ERROR: Buffer overflow

		isize bytesRead = ::read(reader.fd, data + reader.size, bytes);
		if (bytesRead < 0)
			return -2;
		reader.size += (usize) bytesRead;	// TODO: what do we do on failures?
		return bytesRead;
	}

	isize write(usize bytes) {
		if (writer.index == writer.size)
			return 0;	// Nothing to write, should be an error if the payload isnt 0

		void *src = data + sizeof(data) / 2 + writer.index;
		bytes = MIN(bytes, writer.size - writer.index);
		isize bytesWritten = ::write(writer.fd, src, bytes);	// TODO: The write here reads from a different FD, and from a different buffer too
		if (bytesWritten < 0)
			return -1;
		return bytesWritten;
	}

	isize readAll(usize bytes) {
		if (reader.size + bytes > sizeof(data))
			return -1;	// ERROR: Buffer overflow

		isize bytesRead = ::read(reader.fd, data + reader.size, bytes);
		if (bytesRead < 0)
			return -2;
		reader.size += (usize) bytesRead;	// TODO: what do we do on failures?
		return bytesRead;	
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

	u8 *getRead() {
		return data + reader.index;
	}

	u8 *getWrite() {
		return data + writer.index;
	}
};
