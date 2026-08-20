#pragma once

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <new>
#include "core.hpp"
#include "HTTP.hpp"
#include <vector>
#include <string>
#include "Parser_helpers.ipp"

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
		std::string value;
	};

	struct Directive {
		std::string name;
		std::vector<std::string> args;
	};

	typedef std::vector<Token>::const_iterator tokIter;

public:
	char* fileBuffer;
	usize fileSize;

	int cleanup() {
		delete[] fileBuffer;
		return 1;
	}

	usize get_word(char* &str, std::string &word);
	void match_delimiter(std::string &word, usize &delimPos, isize &braces, Token &token);
	std::vector<Token> tokenizer(char *fileBuffer);

	void match(tokIter &cursor, const std::string &value);
	void advance(tokIter &cursor, tokIter &end);
	usize find_scope_end(tokIter &begin, tokIter &end);

	void set_methods(std::vector<std::string> &methods, HTTP::Location &location);
	isize set_location_directive(Directive &dir, HTTP::Location &location);
	isize set_server_directive(Directive &dir, HTTP::ServerConfig &server);
	isize parse_directive(tokIter &cursor, tokIter &end, Directive &dir);
	isize parse_location(tokIter &cursor, tokIter &end, HTTP::Location &loc);
	isize parse_server(tokIter cursor, tokIter end, HTTP::ServerConfig &server);
	std::vector<ServerConfig> parse_config(char *fileBuffer, usize totalBytes);

	Parser(const char *filePath) {
		int fd = open(filePath, O_RDONLY);
		if (fd == -1)
			PERR_EXIT(1, "Error: Failed to open file");

		struct stat st;
		if (fstat(fd, &st) == -1 || st.st_size < 16) {	// TODO: might remove this failure path
			close(fd);
			PERR_EXIT(1, "Error: Invalid file");
		}

		fileSize = (usize) st.st_size;
		usize allocSize = ALIGN_UP(fileSize + 63, (usize)64);	// Pads with at least 64 bytes

		fileBuffer = new (std::nothrow) char[allocSize];
		if (fileBuffer == NULL) {
			close(fd);
			PERR_EXIT(1, "Error: Allocation failure");
		}

		usize curBytes = 0;
		while (curBytes < fileSize) {
			usize bytesRemaining = fileSize - curBytes;
			isize bytesRead = read(fd, fileBuffer + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
			if (bytesRead <= 0) {
				close(fd);
				delete[] fileBuffer;
				PERR_EXIT(1, "Error: Read failure");
			}
			curBytes += (usize) bytesRead;
		}
		close(fd);
		fileBuffer[fileSize] = 0;
		fileBuffer[fileSize + 1] = '{';
		fileBuffer[fileSize + 2] = '}';
		fileBuffer[fileSize + 3] = ';';		
	}
};
}

#include "Parser_tokens.ipp"
#include "Parser_config.ipp"