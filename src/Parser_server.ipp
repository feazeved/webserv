#pragma once
#include "core.hpp"
#include "HTTP.hpp"
#include "Parser.hpp"
#include "Parser_helpers.ipp"

namespace HTTP {
//

PARSER_INL
(isize) parse_directive(const Array32<Token> &tokens, usize &cursor, usize end, Directive &dir) {
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(1, "Error: Unexpected token");
	dir.name = tokens[cursor].value;
	cursor++;
	usize argumentStart = cursor;
	while (cursor != end && tokens[cursor].type == Token::WORD)
		cursor++;
	if (cursor == end || tokens[cursor].type != Token::SEMICOLON)
		PERR_EXIT(1, "Error: Unexpected token");
	if (dir.args.alloc((u32)(cursor - argumentStart)) == true)
		_exit(1);
	for (u32 index = 0; index < dir.args.count; index++)
		dir.args[index] = tokens[argumentStart + index].value;
	return 0;
}

PARSER_INL
(isize) set_server_directive(Directive &dir, VirtualServer &server) {
	if (dir.name == "listen") {
		if (server.port != SIZE_MAX || dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid port definition");
		const StringView &listen = dir.args[0];
		const char *port = listen.get();
		usize portLength = listen.length;
		char *separator = (char*)MEMCHR(port, ':', portLength);
		if (separator != NULL) {
			usize hostLength = (usize)(separator - port);
			if (hostLength == 0 || hostLength == listen.length - 1
				|| server.host.length != 0)
				PERR_EXIT(1, "Error: Invalid listen address");
			server.host = StringView((u32)hostLength, listen.offset);
			*separator = '\0';
			port = separator + 1;
			portLength -= hostLength + 1;
		}
		server.port = s_strtol10(port, portLength);
		if (server.port < 1 || server.port > 65535)
			PERR_EXIT(1, "Error: Invalid port");
	}
	else if (dir.name == "host") {
		if (server.host.length != 0 || dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid host definition");
		server.host = dir.args[0];
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid max body size definition");
		server.maxBodySize = s_strtol10(dir.args[0].get(), dir.args[0].length);
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			PERR_EXIT(1, "Error: Invalid max body size");
		server.maxBodySize <<= 20;	// TODO: add M check for size
	}
	else if (dir.name == "error_page") {
		if (dir.args.count < 2)
			PERR_EXIT(1, "Error: Invalid error page");
		StringView path = dir.args[dir.args.count - 1];
		for (u32 index = 0; index + 1 < dir.args.count; index++) {
			usize error = s_strtol10(dir.args[index].get(), dir.args[index].length);
			const bool validError = (error >= 400 && error <= 431) || (error >= 500 && error <= 511);
			if (dir.args[index].length != 3 || !validError)
				PERR_EXIT(1, "Error: Invalid error number");
			if (error < 500)
				server.clientErrors[error - 400] = path;
			else
				server.serverErrors[error - 500] = path;
		}
	}
	else
		PERR_EXIT(1, "Error: Invalid server directive");
	return 0;
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
	if (server.locations.alloc(locationCount) == true)
		_exit(1);

	usize locationIndex = 0;
	while (cursor != end) {
		if (tokens[cursor].value == "location") {
			HTTP::Location loc;
			parse_location(tokens, cursor, end, loc);
			for (usize index = 0; index < locationIndex; index++) {
				const StringView &path = server.locations[index].url;
				if (path.length == loc.url.length && MEMCMP(path.get(), loc.url.get(), path.length) == 0)
					PERR_EXIT(1, "Error: Duplicate location");
			}
			server.locations[locationIndex] = loc;
			locationIndex++;
		}
		else {
			Directive dir;
			parse_directive(tokens, cursor, end, dir);
			set_server_directive(dir, server);
		}
		cursor++;
	}
	if (tokens[end].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	if (server.port == SIZE_MAX)
		PERR_EXIT(1, "Error: Missing listen directive");
	if (server.host.length == 0)
		server.host = StringView(sizeof("localhost") - 1, (u32)(fileOffset + fileSize + 4));
	return 0;
}

PARSER_INL
(void) parse_file(VirtualServer (&servers)[MAX_VIRTUAL_SERVERS]) {
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
}

//
}	// Namespace HTTP
