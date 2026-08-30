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
		for (usize serverIndex = 0; serverIndex < serverCount; serverIndex++) {
			tokArray.ptr++;
			parse_server(tokArray, servers[serverIndex]);
		}
		for (usize index = 0; index < serverCount; index++) {
			cache_error_pages(servers[index]);
		}
		alpha.clear();
	}

	Array<Token> tokenize();
	void cache_error_pages(VirtualServer &server);
	void parse_location(Array<Token> &tokArray, Location &loc, const Array<Location> &locations);
	void parse_server(Array<Token> &tokArray, VirtualServer &server);

	Span32 parse_cgi(Array<Token> &tokArray, const Array<Location> &locations);
	void parse_location_directive(Location &location, Directive &dir, const Array<Location> &locations);
	void parse_server_directive(VirtualServer &server, Directive &dir);
	static Directive s_build_directive(Arena &arena, Array<Token> &tokArray);
	static bool s_read_whole_file(Arena &arena, const char *filePath, Span &file, usize padSize = 32, usize minSize = 0, usize maxSize = UINT32_MAX);
};

#include "Parser_common.ipp"
#include "Parser_locations.ipp"
#include "Parser_server.ipp"
#include "Parser_tokenize.ipp"
#include "Parser_process.ipp"
