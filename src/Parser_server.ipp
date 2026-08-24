#pragma once
#include "core.hpp"
#include "HTTP.hpp"
#include "Parser.hpp"
#include "Parser_helpers.ipp"

namespace HTTP {
//

static inline
void s_directive_error_page(Directive &dir, VirtualServer &server) {
	if (dir.args.count < 2)
		PERR_EXIT(1, "Error: Invalid error page");
	StringView32 path = dir.args[dir.args.count - 1];
	if (s_length_check(path.length))
		PERR_EXIT(1, "Error: Invalid error page");
	for (u32 index = 0; index + 1 < dir.args.count; index++) {
		usize error = s_strtol10(dir.args[index].kptr(), dir.args[index].length);
		Status status(error);
		if (dir.args[index].length != 3 || !status.is_error())
			PERR_EXIT(1, "Error: Invalid error number");
		server.errorPages[status.get_page_index()] = path;
	}
}

static inline
void s_directive_listen(Directive &dir, VirtualServer &server) {
	if (server.port != SIZE_MAX || dir.args.count != 1)
		PERR_EXIT(1, "Error: Invalid port definition");
	const StringView32 &listen = dir.args[0];
	const char *port = listen.kptr();
	usize portLength = listen.length;
	char *separator = (char*)MEMCHR(port, ':', portLength);
	if (separator != NULL) {
		usize hostLength = (usize)(separator - port);
		if (hostLength == 0 || hostLength == listen.length - 1
			|| server.host.length != 0)
			PERR_EXIT(1, "Error: Invalid listen address");
		server.host = StringView32((u32)hostLength, listen.offset);
		*separator = '\0';
		port = separator + 1;
		portLength -= hostLength + 1;
	}
	server.port = s_strtol10(port, portLength);
	if (server.port < 1 || server.port > 65535)
		PERR_EXIT(1, "Error: Invalid port");
}

static inline
void s_directive_body_size(const StringView32 &value, usize &bodySize, VirtualServer &server) {
	if (server.maxBodySize != SIZE_MAX || value.length == 0)
		PERR_EXIT(1, "Error: Invalid max body size");

	u8 factor = 0;
	const char *str = value.kptr();
	usize digitLength = value.length;
	if (str[digitLength - 1] == 'G') {
		factor = 30;
		digitLength--;
	}
	else if (str[digitLength - 1] == 'M') {
		factor = 20;
		digitLength--;
	}
	else if (str[digitLength - 1] == 'K') {
		factor = 10;
		digitLength--;
	}

	const usize bytes = s_strtol10(str, digitLength);
	if (bytes == SIZE_MAX || bytes > (SIZE_MAX >> factor))
		PERR_EXIT(1, "Error: Invalid max body size");
	bodySize = bytes << factor;
}

static inline
void s_parse_server_directive(VirtualServer &server, const Array32<Token> &tokens, usize &cursor, usize end) {
	Directive dir = s_build_directive(tokens, cursor, end);

	if (dir.name == "error_page")
		return s_directive_error_page(dir, server);
	if (dir.args.count != 1 || s_length_check(dir.args[0].length))
		PERR_EXIT(1, "Error: Invalid server directive");
	if (dir.name == "listen")
		return s_directive_listen(dir, server);
	if (dir.name == "host")
		server.host = dir.args[0];
	else if (dir.name == "client_max_body_size")
		s_directive_body_size(dir.args[0], server.maxBodySize, server);
	else if (dir.name == "root")
		server.serverRoot = dir.args[0];
	else
		PERR_EXIT(1, "Error: Invalid server directive");
}

PARSER_INL
(isize) parse_server(const Array32<Token> &tokens, usize cursor, usize end, VirtualServer &server) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "server"))
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type == Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Empty server block");

	usize locationCount = s_count_locations(tokens, cursor, end);
	if (server.locations.alloc_b(locationCount) == true)
		_exit(1);

	usize locationIndex = 0;
	while (cursor != end) {
		if (tokens[cursor].value == "location") {
			HTTP::Location loc;
			parse_location(tokens, cursor, end, loc);
			for (usize index = 0; index < locationIndex; index++) {
				const StringView32 &path = server.locations[index].url;
				if (path.length == loc.url.length && MEMCMP(path.kptr(), loc.url.kptr(), path.length) == 0)
					PERR_EXIT(1, "Error: Duplicate location");
			}
			server.locations[locationIndex] = loc;
			locationIndex++;
		}
		else
			s_parse_server_directive(server, tokens, cursor, end);
		cursor++;
	}
	if (tokens[end].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	if (server.port == SIZE_MAX)
		PERR_EXIT(1, "Error: Missing listen directive");
	if (server.host.length == 0)
		server.host = StringView32(sizeof("localhost") - 1, (u32)(fileOffset + fileSize + 4));
	return 0;
}

//
}	// Namespace HTTP
