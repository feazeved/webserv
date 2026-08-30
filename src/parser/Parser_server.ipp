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
	if (dir.args.count != 1 || s_length_check(dir.args[0].length))
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
}

static inline
usize s_count_locations(Array<Parser::Token> tokArray) {
	usize locationCount = 0;
	while (tokArray[0].type != Parser::Token::CLOSE_BRACKET) {
		if (tokArray[0].value == "location") {
			Parser::Token *locationStart = tokArray.ptr;
			tokArray.ptr++;
			if (tokArray[0].type != Parser::Token::WORD || tokArray[1].type != Parser::Token::OPEN_BRACKET)
				PERR_EXIT(1, "Error: Invalid location block");
			tokArray.ptr += 2;
			usize braces = 1;
			while (braces != 0) {
				braces += tokArray[0].type == Parser::Token::OPEN_BRACKET;
				braces -= tokArray[0].type == Parser::Token::CLOSE_BRACKET;
				tokArray.ptr++;
			}
			const usize locationSize = (usize)(tokArray.ptr[-1].value.ptr - locationStart->value.ptr) + 1;
			if (locationSize <= 1 || locationSize > MAX_LOCATION_BLOCK_SIZE)
				PERR_EXIT(1, "Error: Invalid location block");	// No empty locations either
			locationCount++;
		}
		else {
			while (tokArray[0].type == Parser::Token::WORD)
				tokArray.ptr++;
			if (tokArray[0].type != Parser::Token::SEMICOLON)
				PERR_EXIT(1, "Error: Unexpected token");
			tokArray.ptr++;
		}
	}
	if (locationCount >= UINT16_MAX)
		PERR_EXIT(1, "Error: More than 65535 locations");
	return locationCount;
}

PARSER_INL
(void) parse_server(Array<Token> &tokArray, VirtualServer &server) {
	tokArray.ptr++;
	usize locationCount = s_count_locations(tokArray);
	Array<ParsedLocation> parsedLocations = alpha.alloc_array<ParsedLocation>(locationCount);
	if (parsedLocations.ptr == NULL)
		std::exit(1);

	usize locationIndex = 0;
	while (tokArray[0].type != Token::CLOSE_BRACKET) {
		if (tokArray[0].value == "location") {
			tokArray.ptr++;
			ParsedLocation loc = parse_location(tokArray);
			for (usize index = 0; index < locationIndex; index++) {
				const Span &path = parsedLocations[index].uri;
				if (path.length == loc.uri.length && MEMCMP(path.ptr, loc.uri.ptr, path.length) == 0)
					PERR_EXIT(1, "Error: Duplicate location");
			}
			parsedLocations[locationIndex] = loc;
			locationIndex++;
		}
		else {
			Directive dir = s_build_directive(alpha, tokArray);
			parse_server_directive(server, dir);
		}
	}
	tokArray.ptr++;
	if (server.port == SIZE_MAX)
		PERR_EXIT(1, "Error: Missing listen directive");
	if (server.host.length == 0)
		server.host = beta.copy_span(Span::create("localhost"));
	server.locations = store_locations(parsedLocations);
}
