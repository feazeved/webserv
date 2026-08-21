#pragma once

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "core.hpp"
#include "HTTP.hpp"
#include "Parser_helpers.ipp"
#include "Arena.hpp"

namespace HTTP {

// TODO: Find a way to free tokens and directives
// TODO: Change count_locations to be static inside helpers

#define PARSER_INL(ret_type) ret_type inline HTTP::Parser::

class Parser {
public:
	struct Token {
		enum Type {
			OPEN_BRACKET,
			CLOSE_BRACKET,
			SEMICOLON,
			WORD
		}	type;
		StringView value;
	};

	struct Directive {
		StringView name;
		Array32<StringView> args;
	};

public:
	usize fileOffset;
	usize fileSize;
	usize serverCount;

	char* getPtr() {
		return fileOffset + (char*) Arena::data;
	}

	int cleanup() {
		Arena::clear();
		fileSize = 0;
		return 1;
	}

	usize get_next_word(char* &ostr);
	Token match_delimiter(char *ptr, usize delimPos, isize &braces);
	Array32<Token> tokenize();

	usize find_scope_end(const Array32<Token> &tokens, usize begin, usize end);
	usize count_locations(const Array32<Token> &tokens, usize cursor, usize end);

	void set_methods(Array32<StringView> &methods, HTTP::Location &location);
	isize set_location_directive(Directive &dir, HTTP::Location &location);
	isize set_server_directive(Directive &dir, HTTP::ServerConfig &server);
	isize parse_directive(const Array32<Token> &tokens, usize &cursor, usize end, Directive &dir);
	isize parse_location(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc);
	isize parse_server(const Array32<Token> &tokens, usize cursor, usize end, HTTP::ServerConfig &server);
	void parse_config(ServerConfig (&servers)[MAX_VIRTUAL_SERVERS]);

	Parser(const char *filePath) : fileOffset(0), fileSize(0), serverCount(0) {
		int fd = open(filePath, O_RDONLY);
		if (fd == -1)
			PERR_EXIT(1, "Error: Failed to open file");

		struct stat st;
		if (fstat(fd, &st) == -1 || st.st_size < 16 || (u64)st.st_size > (u64)UINT32_MAX - 127) {
			close(fd);
			PERR_EXIT(1, "Error: Invalid file");
		}

		fileSize = (usize) st.st_size;
		usize allocSize = ALIGN_UP(fileSize + 63, (usize)64);	// Pads with at least 64 bytes

		fileOffset = Arena::alloc_index(allocSize);
		if (fileOffset == UINT32_MAX) {
			close(fd);
			_exit(1);
		}

		char* ptr = getPtr();
		usize curBytes = 0;
		while (curBytes < fileSize) {
			usize bytesRemaining = fileSize - curBytes;
			isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
			if (bytesRead <= 0) {
				close(fd);
				Arena::clear();
				PERR_EXIT(1, "Error: Read failure");
			}
			curBytes += (usize) bytesRead;
		}
		close(fd);
		ptr[fileSize] = 0;
		ptr[fileSize + 1] = '{';
		ptr[fileSize + 2] = '}';
		ptr[fileSize + 3] = ';';
		MEMCPY_INLINE(ptr + fileSize + 4, "localhost", sizeof("localhost"));

		serverCount = s_count_servers(ptr, fileSize);
		if (serverCount > MAX_VIRTUAL_SERVERS)
			PERR_EXIT(cleanup(), "Error: Invalid config");
	}
};
}

#include "Parser_tokens.ipp"
#include "Parser_config.ipp"
