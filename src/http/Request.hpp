#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "core.hpp"
#include "Request_helpers.inl"
#include "http/Buffer.hpp"

// Implement a compact function so that if necessary, moves cookie to the end of query so that path + query + cookie < 8192

// Implement state actions to Request so that it switches between reading and writing seamlessly
	// It needs to set the epoll variables and confirm upon entry that it can write
	// With chunked transfer encoding, it will constantly switch between read and write

// Implement method actions to Request so that it can GET, POST, DELETE, CGI or ERROR
	// This involves returning a HTTP Header with the appropriate status code and the payload

// Create a separate Parser class that handles the reading of the header only
	// It is supposed to output a struct containing all of the relevant metadata
	// It does not own the buffer

// Whenever status is deduced, it already loads the string into output buffer index 9 e.g. (HTTP/1.1 _)

// Fake envp added to config for easy insertion

namespace HTTP {

namespace Attributes {

enum Attributes {
	METHOD_GET = 1 << 0,
	METHOD_POST = 1 << 1,
	METHOD_DELETE = 1 << 2,
	CGI = 1 << 3,
	HOST = 1 << 4,
	CHUNKED = 1 << 5,
	DONE = 1 << 7
};

enum State {
	READING = 1 << 0,		// Reading header
	PROCESSING = 1 << 1,	// Reading body
	WRITING = 1 << 2,		// Writing

	PROCESSING_LENGTH = 1 << 6, // Reading the header for the body
	FIRST_LINE = 1 << 7
};
}

typedef struct {
	struct {
		u32	index;
		u32 size;
	}	path, query, cookie;
}	RequestVars;

// 3 Stages:
// 1) Read and decode information from client (means already dechunkify)
// 2) a) Write to an FD (POST or CGI)
// 2) b) Read from an FD (GET or DEL)
// 3) Write to client (Response + optionally (Get or CGI))

template <usize bufferSize>
class Request {
public:
	u8 type; // bitfield: (DONE) (-) (CHUNKED) (HOST) (CGI) (DELETE) (POST) (GET)
	u8 state;	// TODO: transfer all of these to a metadata struct
	u32 status;
	RequestVars vars;
	u32 lineStart;
	usize bodySize, chunkSize;
	Buffer<bufferSize> clientInput, clientOutput;
	Buffer<bufferSize> serverInput, serverOutput;
	bool chunkDone;

// Methods
isize configure() {
	static const u8 transferCheck = HTTP::Attributes::CHUNKED | HTTP::Attributes::METHOD_POST;

	// Error checking
	if (status != 0) {	// An error caused early interruption
		// buildHeader();
		// Close the connection
	}

	if ((type & HTTP::Attributes::HOST) == 0) {	// Host wasnt set
		// Set status
	}

	// if ((type & transferCheck) == transferCheck) {
	// 	if (bodySize == SIZE_MAX) {	// TODO: needs checking, conditions aint right
	// 		return -1;	// Set status, transfer encoding was not set
	// 	}
	// 	source = &server.writer;
	// }
	// else
	// 	source = &server.reader;

}

// TODO: This is wrong, it's always hexadecimal
isize readDigitLine(Buffer<bufferSize>& src) {
	u32 lineEnd = src.find_line_end();
	if (lineEnd == UINT32_MAX)
		return 1;

	chunkSize = s_strtol16(src.data + lineStart);
	if (chunkSize == SIZE_MAX)
		return -1;
	lineStart = lineEnd;
	if (chunkSize == 0)
		chunkDone = true;
	return 0;
}

isize dechunk(usize bytes, Buffer<bufferSize>& src, i32 targetFd) {
	Buffer<bufferSize> buffer(targetFd);

	u32 lineEnd = UINT32_MAX;
	isize rvalue = 0;

	while (true) {
		if (chunkSize == SIZE_MAX) {
			rvalue = readDigitLine(src);
			if (rvalue == -1)
				return rvalue;
			else if (rvalue == 1)
				break;
		}
		else if (chunkSize > 0) {
			usize bytesAppended = buffer.append(src, chunkSize);
			chunkSize -= bytesAppended;	// Guaranteed to be chunksize or less
			if (src.index == src.size)	// Need to read from client for more info
				break;
		}
		else {
			lineEnd = src.find_line_end();
			if (lineEnd == UINT32_MAX)
				break;
			if (lineEnd != 0)
				return -1;	// There was no \r\n after 0, or after a message
			if (chunkDone == true)
				break;
			chunkSize = SIZE_MAX;
		}
	}

	if (buffer.index == 0)
		return 0;

	isize bytesWritten = buffer.write(bytes);
	if (bytesWritten < 0)
		return bytesWritten;

	// Optimization opportunity here to have src copy directly to itself
	if (src.index < src.size) {
		isize bytesRemaining = src.size - src.index;
		buffer.append(src, (usize)bytesRemaining);
	}
	if (buffer.size > buffer.index) {
		src.reset();
		src.append(buffer, SIZE_MAX);
	}

	return 0;
}

isize read_from_client(usize bytes, u32 events) {
	if (!(type & (HTTP::Attributes::METHOD_POST | HTTP::Attributes::CGI)))
		return read_from_server();

	return write_to_server(clientOutput);
}

isize read_from_client_chunked(usize bytes, u32 events) {
	if (!(type & (HTTP::Attributes::METHOD_POST | HTTP::Attributes::CGI)))
		return read_from_server();

	if (dechunk(clientOutput, serverInput))
		return -1;

	return write_to_server(serverInput);	// dispatches to 
}

// POST or CGI
isize write_to_server(usize bytes, u32 events, Buffer<bufferSize>& source) {
	source.write(serverInput.fd, bytes);	// In this case the FD for client_writer will not be the client
	if (type & HTTP::Attributes::CGI)
		return read_from_server(bytes, events);	// Populate server output
	write_to_client(bytes, events);
}

// GET or DEL
// Needs no body, source from this is server file fd
isize read_from_server(usize bytes, u32 events) {
	serverOutput.read();
	write_to_client(bytes, events);
}

// Source of this will always be server.write buffer
isize write_to_client(usize bytes, u32 events) {
	// Write header (ONCE) wait for CGI
	// Copy from serverOutput to clientInput
	clientInput.write(bytes, events);
}

// isize buildHeader() {
// 	client.append("HTTP/1.1 ");

// 	if (bodySize != SIZE_MAX)
// 		client.append("Transfer-Encoding: chunked\r\n");
// 	else
// 	{
// 		client.append("Content-Length: ");
// 		client.append(requestSize, false);	// Auto performs itoa
// 	}

// 	// Other lines here
// 	// Location
// 	// Content Type
// 	// Content Encoding?

// 	client.append("\r\n");
// 	// if (isBad(status)) {
// 	// 	client.append(client.data + 9, statusEnd - 9);
// 	// 	return;
// 	// }
// }

// ======== Constructors ====================
Request() :
	type(0),
	state(0),
	bodySize(SIZE_MAX) {
	}
};
}
