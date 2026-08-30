#pragma once
#include <fcntl.h>
#include <sys/stat.h>

#include "core.hpp"
#include "Array.hpp"
#include "Arena.hpp"
#include "Span.hpp"
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
		Span value;
	};

	struct Directive {
		Span name;
		Array<Span> args;
	};

	Arena &alpha;
	Arena &beta;
	Span file;
	usize serverCount;

	Parser(const char *filePath, VirtualServer (&servers)[MAX_VIRTUAL_SERVERS], Arena &srcAlpha, Arena &srcBeta)
		: alpha(srcAlpha), beta(srcBeta), file(), serverCount(0) {
		if (s_read_whole_file(alpha, filePath, file, 63, 16))
			std::exit(1);
		Array<Token> tokArray = tokenize();
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
		for (usize index = 0; index < serverCount; index++) {
			cache_error_pages(servers[index]);
		}
		alpha.clear();
	}

	Array<Token> tokenize();
	void cache_error_pages(VirtualServer &server);
	isize parse_location(const Array<Token> &tokens, usize &cursor, usize end, Location &loc, const Array<Location> &locations);
	isize parse_server(const Array<Token> &tokens, usize cursor, usize end, VirtualServer &server);

	Span32 parse_cgi(const Array<Token> &tokens, usize &cursor, usize end, const Array<Location> &locations);
	void parse_location_directive(Location &location, Directive &dir, const Array<Location> &locations);
	void parse_server_directive(VirtualServer &server, Directive &dir);
	static Directive s_build_directive(Arena &arena, const Array<Token> &tokens, usize &cursor, usize end);
	static bool s_read_whole_file(Arena &arena, const char *filePath, Span &file, usize padSize = 32, usize minSize = 0, usize maxSize = UINT32_MAX);
	static usize s_find_scope_end(const Array<Token> &tokens, usize begin, usize end);
};

#include "Parser_common.ipp"
#include "Parser_locations.ipp"
#include "Parser_server.ipp"
#include "Parser_tokenize.ipp"
#include "Parser_process.ipp"
