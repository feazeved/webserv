#pragma once
#include "Parser.hpp"

static inline
void s_directive_error_page(Parser::Directive &dir, VirtualServer &server) {
	if (dir.args.count < 2)
		PERR_EXIT(1, "Error: Invalid error page");
	Span path = dir.args[dir.args.count - 1];
	if (s_length_check(path.length))
		PERR_EXIT(1, "Error: Invalid error page");
	for (usize index = 0; index + 1 < dir.args.count; index++) {
		usize error = s_strtol10(dir.args[index].ptr, dir.args[index].length);
		Status status(error);
		if (dir.args[index].length != 3 || !status.is_error())
			PERR_EXIT(1, "Error: Invalid error number");
		server.errorPages[status.get_page_index()] = path;
	}
}

static inline
void s_directive_listen(Arena &arena, const Span &value, VirtualServer &server) {
	if (server.port != SIZE_MAX)
		PERR_EXIT(1, "Error: Invalid port definition");
	const char *port = value.ptr;
	usize portLength = value.length;
	char *separator = (char*)MEMCHR(port, ':', portLength);
	if (separator != NULL) {
		usize hostLength = (usize)(separator - port);
		if (hostLength == 0 || hostLength == value.length - 1 || server.host.length != 0)
			PERR_EXIT(1, "Error: Invalid listen address");
		Span host = {(char*)port, hostLength};
		server.host = arena.copy_span(host);
		port = separator + 1;
		portLength -= hostLength + 1;
	}
	server.port = s_strtol10(port, portLength);
	if (server.port < 1 || server.port > 65535)
		PERR_EXIT(1, "Error: Invalid port");
}

static inline
void s_directive_body_size(const Span &value, usize &bodySize) {
	if (bodySize != SIZE_MAX || value.length == 0)
		PERR_EXIT(1, "Error: Invalid max body size");

	u8 factor = 0;
	const char *str = value.ptr;
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
	if (digitLength == 0)
		PERR_EXIT(1, "Error: Invalid max body size");

	const usize bytes = s_strtol10(str, digitLength);
	if (bytes == SIZE_MAX || bytes > (SIZE_MAX >> factor))
		PERR_EXIT(1, "Error: Invalid max body size");
	bodySize = bytes << factor;
}

PARSER_INL
(void) parse_server_directive(VirtualServer &server, Directive &dir) {
	if (dir.name == "error_page")
		return s_directive_error_page(dir, server);
	if (dir.args.count != 1)
		PERR_EXIT(1, "Error: Invalid server directive");
	const Span &value = dir.args[0];
	if (dir.name == "listen")
		return s_directive_listen(beta, value, server);
	else if (dir.name == "host") {
		if (server.host.length != 0)
			PERR_EXIT(1, "Error: Duplicate host definition");
		server.host = beta.copy_span(value);
	}
	else if (dir.name == "client_max_body_size")
		s_directive_body_size(value, server.maxBodySize);
	else if (dir.name == "root") {
		if (server.serverRoot.length != 0)
			PERR_EXIT(1, "Error: Duplicate root definition");
		server.serverRoot = beta.copy_span(value);
	}
	else
		PERR_EXIT(1, "Error: Invalid server directive");
	if (s_length_check(value.length))
		PERR_EXIT(1, "Error: Invalid server directive");	// Review why not above
}

static inline
usize s_count_locations(const Array<Parser::Token> &tokens, usize cursor, usize end) {
	usize locationCount = 0;
	usize locationSize;

	while (cursor < end) {
		if (tokens[cursor].value == "location") {
			usize distance = Parser::s_find_scope_end(tokens, cursor, end);
			locationSize = (usize)(tokens[cursor + distance].value.ptr - tokens[cursor].value.ptr) + 1;
			if (locationSize > MAX_LOCATION_BLOCK_SIZE)
				PERR_EXIT(1, "Error: Location block exceeds maximum size");
			locationCount++;
			cursor += distance + 1;
		}
		else {
			while (cursor < end && tokens[cursor].type != Parser::Token::SEMICOLON)
				cursor++;
			if (cursor < end)
				cursor++;
		}
	}
	if (locationCount >= UINT16_MAX)
		PERR_EXIT(1, "Error: More than 65535 locations");
	return locationCount;
}

PARSER_INL
(isize) parse_server(const Array<Token> &tokens, usize cursor, usize end, VirtualServer &server) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "server"))
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type == Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Empty server block");

	usize locationCount = s_count_locations(tokens, cursor, end);
	server.locations = beta.alloc_array<Location>(locationCount);
	if (server.locations.ptr == NULL)
		std::exit(1);
	Span emptySource;
	emptySource.ptr = (char*)"";
	emptySource.length = 0;
	const Span32 empty = beta.compress_span(server.locations, emptySource);

	usize locationIndex = 0;
	while (cursor != end) {
		if (tokens[cursor].value == "location") {
			Location loc(empty);
			parse_location(tokens, cursor, end, loc, server.locations);
			for (usize index = 0; index < locationIndex; index++) {
				const Span path = server.locations.extract(server.locations[index].uri);
				const Span candidate = server.locations.extract(loc.uri);
				if (path.length == candidate.length
					&& MEMCMP(path.ptr, candidate.ptr, path.length) == 0)
					PERR_EXIT(1, "Error: Duplicate location");
			}
			server.locations[locationIndex] = loc;
			locationIndex++;
		}
		else {
			Directive dir = s_build_directive(alpha, tokens, cursor, end);
			parse_server_directive(server, dir);
		}
		cursor++;
	}
	if (tokens[end].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	if (server.port == SIZE_MAX)
		PERR_EXIT(1, "Error: Missing listen directive");
	if (server.host.length == 0) {
		Span localhost;
		localhost.ptr = (char*)"localhost";
		localhost.length = sizeof("localhost") - 1;
		server.host = beta.copy_span(localhost);
	}
	return 0;
}
