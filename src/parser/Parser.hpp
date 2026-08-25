#pragma once
#include "core.hpp"
#include "webserv.hpp"
#include "Array.hpp"
#include "VirtualServer.hpp"
#include "Parser_helpers.ipp"

#define PARSER_INL(ret_type) ret_type inline Parser::

class Parser {

public:
	usize fileOffset;
	usize fileSize;
	usize serverCount;

	Parser(const char *filePath, VirtualServer (&servers)[MAX_VIRTUAL_SERVERS]) {
		if (s_read_whole_file(filePath, fileSize, fileOffset, 63))
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
};

#include "Parser_locations.ipp"
#include "Parser_server.ipp"
#include "Parser_tokenize.ipp"
#include "Parser_process.ipp"
