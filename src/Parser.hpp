#pragma once

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "core.hpp"
#include "HTTP.hpp"
#include <vector>
#include "Parser_helpers.ipp"
#include "Arena.hpp"

namespace HTTP {
//

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
		std::vector<StringView> args;
	};

	typedef std::vector<Token>::const_iterator tokIter;

public:
	char* fileBuffer;
	usize fileSize;

	int cleanup() {
		Arena::clear();
		fileSize = 0;
		return 1;
	}

	usize get_next_word(char* &ostr);
	Token match_delimiter(char *ptr, usize delimPos, isize &braces);
	std::vector<Token> tokenize();

	usize find_scope_end(tokIter &begin, tokIter &end);

	void set_methods(std::vector<StringView> &methods, HTTP::Location &location);
	isize set_location_directive(Directive &dir, HTTP::Location &location);
	isize set_server_directive(Directive &dir, HTTP::ServerConfig &server);
	isize parse_directive(tokIter &cursor, tokIter &end, Directive &dir);
	isize parse_location(tokIter &cursor, tokIter &end, HTTP::Location &loc);
	isize parse_server(tokIter cursor, tokIter end, HTTP::ServerConfig &server);
	std::vector<ServerConfig> parse_config();

	Parser(const char *filePath) : fileBuffer(NULL), fileSize(0) {
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

		fileBuffer = (char*) Arena::alloc(allocSize);
		if (fileBuffer == NULL) {
			close(fd);
			_exit(1);
		}

		usize curBytes = 0;
		while (curBytes < fileSize) {
			usize bytesRemaining = fileSize - curBytes;
			isize bytesRead = read(fd, fileBuffer + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
			if (bytesRead <= 0) {
				close(fd);
				Arena::clear();
				PERR_EXIT(1, "Error: Read failure");
			}
			curBytes += (usize) bytesRead;
		}
		close(fd);
		fileBuffer[fileSize] = 0;
		fileBuffer[fileSize + 1] = '{';
		fileBuffer[fileSize + 2] = '}';
		fileBuffer[fileSize + 3] = ';';
		MEMCPY_INLINE(fileBuffer + fileSize + 4, "localhost", sizeof("localhost"));
	}
};
}

#include "Parser_tokens.ipp"
#include "Parser_config.ipp"
