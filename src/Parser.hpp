#pragma once
#include <sys/stat.h>

#include "core.hpp"
#include "HTTP.hpp"
#include "VirtualServer.hpp"
#include "Parser_helpers.ipp"

#define PARSER_INL(ret_type) ret_type inline HTTP::Parser::

namespace HTTP {

class Parser {
public:
	usize fileOffset;
	usize fileSize;
	usize serverCount;

	char* get_ptr() {
		return fileOffset + (char*) Arena::data;
	}

	Parser(const char *filePath) {
		int fd = open(filePath, O_RDONLY);
		if (fd == -1)
			PERR_EXIT(1, "Error: Failed to open file");

		struct stat st;
		if (fstat(fd, &st) == -1 || st.st_size < 16 || (usize)st.st_size > MAX_FILE_SIZE - 127) {
			close(fd);
			PERR_EXIT(1, "Error: Invalid file");
		}

		fileSize = (usize) st.st_size;
		usize allocSize = ALIGN_UP(fileSize + 63, (usize)64);	// Pads with at least 64 bytes
		fileOffset = Arena::alloc_index(allocSize);

		char* ptr = get_ptr();
		usize curBytes = 0;
		while (curBytes < fileSize) {
			usize bytesRemaining = fileSize - curBytes;
			isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
			if (bytesRead <= 0) {
				close(fd);
				PERR_EXIT(1, "Error: Read failure");
			}
			curBytes += (usize) bytesRead;
		}
		close(fd);
	}

	Array32<Token> tokenize();
	void parse_file(VirtualServer (&servers)[MAX_VIRTUAL_SERVERS]);
	isize set_location_directive(Directive &dir, HTTP::Location &location);
	isize set_server_directive(Directive &dir, VirtualServer &server);
	isize parse_directive(const Array32<Token> &tokens, usize &cursor, usize end, Directive &dir);
	isize parse_cgi(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc);
	isize parse_location(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc);
	isize parse_server(const Array32<Token> &tokens, usize cursor, usize end, VirtualServer &server);
};
}

#include "Parser_locations.ipp"
#include "Parser_server.ipp"
#include "Parser_tokenize.ipp"