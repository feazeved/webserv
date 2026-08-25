#pragma once
#include <fcntl.h>
#include <sys/stat.h>

#include "core.hpp"
#include "Array.hpp"
#include "VirtualServer.hpp"

#define PARSER_INL(ret_type) ret_type inline Parser::

class Parser {
public:
	struct Token {
		enum Type {
			OPEN_BRACKET,
			CLOSE_BRACKET,
			SEMICOLON,
			WORD
		}	type;
		StringView32 value;
	};

	struct Directive {
		StringView32 name;
		Array32<StringView32> args;
	};

	usize fileOffset;
	usize fileSize;
	usize serverCount;

	Parser(const char *filePath, VirtualServer (&servers)[MAX_VIRTUAL_SERVERS]) {
		if (s_read_whole_file(filePath, fileOffset, fileSize, 63))
			_exit(1);
		Array32<Token> tokArray = tokenize();
		usize cursor = 0;
		usize end = tokArray.count;
		usize serverIndex = 0;

		while (cursor != end) {
			if (tokArray[cursor].value == "server") {
				usize distance = s_find_scope_end(tokArray, cursor, end);
				parse_server(tokArray, cursor, cursor + distance, servers[serverIndex]);
				serverIndex++;
				cursor += distance;
			}
			else
				PERR_EXIT(1, "Error: Unexpected token");
			cursor++;
		}
		Arena::sizeA = 0;
		for (usize index = 0; index < serverCount; index++) {
			process_cgi_block(servers[index]);
			cache_error_pages(servers[index]);
		}
	}

	Array32<Token> tokenize();
	void cache_error_pages(VirtualServer &server);
	void process_cgi_block(VirtualServer &server);
	isize parse_location(const Array32<Token> &tokens, usize &cursor, usize end, Location &loc);
	isize parse_server(const Array32<Token> &tokens, usize cursor, usize end, VirtualServer &server);

	static Directive s_build_directive(const Array32<Token> &tokens, usize &cursor, usize end);
	static bool s_read_whole_file(const char *filePath, usize &fileOffset, usize &fileSize, usize padSize);
	static usize s_find_scope_end(const Array32<Token> &tokens, usize begin, usize end);
};

#include "Parser_common.ipp"
#include "Parser_locations.ipp"
#include "Parser_server.ipp"
#include "Parser_tokenize.ipp"
#include "Parser_process.ipp"
