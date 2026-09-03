#pragma once
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "core.hpp"
#include "Array.hpp"
#include "Arena.hpp"
#include "Span.hpp"
#include "VirtualServer.hpp"
#include "pure_functions.hpp"

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
		ArrayView<Span> args;
	};

	struct ParsedCgi {
		ArrayView<Token> definitions;
		usize size;

		ParsedCgi() : definitions(), size(0) {}
	};

	struct ParsedLocation {
		Span uri;
		Span root;
		Span index;
		Span uploadStore;
		ParsedCgi cgiBlock;
		Span redirectTarget;
		Status redirectStatus;
		u8 methods;
		bool autoindex;

		ParsedLocation()
			: uri(), root(), index(), uploadStore(), cgiBlock(), redirectTarget(),
			  redirectStatus(), methods(0), autoindex(false) {}
	};

	Arena &alpha;
	Arena &beta;
	Span file;
	usize serverCount;

	Parser(const char *filePath, VirtualServer (&servers)[MAX_VIRTUAL_SERVERS], Arena &srcAlpha, Arena &srcBeta)
		: alpha(srcAlpha), beta(srcBeta), file(), serverCount(0) {
		if (s_read_whole_file(alpha, filePath, file, 63, 16))
			std::exit(1);
		ArrayView<Token> tokArray = tokenize();
		for (usize serverIndex = 0; serverIndex < serverCount; serverIndex++) {
			tokArray.ptr++;
			parse_server(tokArray, servers[serverIndex]);
		}
		for (usize index = 0; index < serverCount; index++) {
			cache_error_pages(servers[index]);
		}
		alpha.clear();
	}

	ArrayView<Token> tokenize();
	void cache_error_pages(VirtualServer &server);
	ParsedLocation parse_location(ArrayView<Token> &tokArray, VirtualServer &server);
	void parse_server(ArrayView<Token> &tokArray, VirtualServer &server);
	ArrayView<Location> store_locations(const ArrayView<ParsedLocation> &source);

	ParsedCgi parse_cgi(ArrayView<Token> &tokArray);
	void parse_location_directive(ParsedLocation &location, Directive &dir);
	void parse_server_directive(VirtualServer &server, Directive &dir);
	static Directive s_build_directive(Arena &arena, ArrayView<Token> &tokArray);
	static bool s_read_whole_file(Arena &arena, const char *filePath, Span &file, usize padSize = 32, usize minSize = 0, usize maxSize = UINT32_MAX);
};

#include "Parser_common.ipp"
#include "Parser_locations.ipp"
#include "Parser_server.ipp"
#include "Parser_tokenize.ipp"
#include "Parser_process.ipp"
