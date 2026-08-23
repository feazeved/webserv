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
		usize error = s_strtol10(dir.args[index].c_str(), dir.args[index].length);
		const bool validError = (error >= 400 && error <= 431) || (error >= 500 && error <= 511);
		if (dir.args[index].length != 3 || !validError)
			PERR_EXIT(1, "Error: Invalid error number");
		if (error < 500)
			server.clientErrors[error - 400] = path;
		else
			server.serverErrors[error - 500] = path;
	}
}

static inline
void s_directive_listen(Directive &dir, VirtualServer &server) {
	if (server.port != SIZE_MAX || dir.args.count != 1)
		PERR_EXIT(1, "Error: Invalid port definition");
	const StringView32 &listen = dir.args[0];
	const char *port = listen.c_str();
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
void s_parse_server_directive(VirtualServer &server, const Array32<Token> &tokens, usize cursor, usize end) {
	Directive dir = s_build_directive(tokens, cursor, end);

	if (dir.name == "error_page")
		return s_directive_error_page(dir, server);
	if (s_length_check(dir.args[0].length))
		PERR_EXIT(1, "Error: Path size is too large");
	if (dir.name == "listen")
		return s_directive_listen(dir, server);
	if (dir.name == "host") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid host definition");
		server.host = dir.args[0];
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid max body size definition");
		server.maxBodySize = s_strtol10(dir.args[0].c_str(), dir.args[0].length);
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			PERR_EXIT(1, "Error: Invalid max body size");
		server.maxBodySize <<= 20;	// TODO: add M check for size
	}
	else if (dir.name == "root") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid server root");
		server.serverRoot = dir.args[0];
	}
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
				if (path.length == loc.url.length && MEMCMP(path.c_str(), loc.url.c_str(), path.length) == 0)
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
