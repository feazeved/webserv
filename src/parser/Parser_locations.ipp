#pragma once
#include "Parser.hpp"

static inline
void s_set_methods(const Array<Span> &methods, Location &location) {
	for (usize index = 0; index < methods.count; index++) {
		if (methods[index] == "GET")
			location.methods |= Options::GET;
		else if (methods[index] == "POST")
			location.methods |= Options::POST;
		else if (methods[index] == "DELETE")
			location.methods |= Options::DELETE;
		else
			PERR_EXIT(1, "Error: Invalid method");
	}
}

PARSER_INL
(void) parse_location_directive(Location &location, Directive &dir, const Array<Location> &locations) {
	usize length = 1;

	if (dir.name == "root") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid root");
		location.root = beta.compress_span(locations, dir.args[0]);
		length = dir.args[0].length;
	}
	else if (dir.name == "autoindex") {
		if (dir.args.count != 1 || (!(dir.args[0] == "on") && !(dir.args[0] == "off")))
			PERR_EXIT(1, "Error: Invalid autoindex");
		location.autoindex = dir.args[0] == "on";
	}
	else if (dir.name == "allowed_methods") {
		if (dir.args.count == 0)
			PERR_EXIT(1, "Error: No allowed methods defined");
		s_set_methods(dir.args, location);
	}
	else if (dir.name == "index") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid index");
		location.index = beta.compress_span(locations, dir.args[0]);
		length = dir.args[0].length;
	}
	else if (dir.name == "upload_store") {
		struct stat st;
		if (dir.args.count != 1 || stat(dir.args[0].ptr, &st) == -1
			|| !S_ISDIR(st.st_mode) || access(dir.args[0].ptr, W_OK | X_OK) == -1)
			PERR_EXIT(1, "Error: Invalid upload store");
		location.uploadStore = beta.compress_span(locations, dir.args[0]);
		length = dir.args[0].length;
	}
	else if (dir.name == "return") {
		if (dir.args.count != 2)
			PERR_EXIT(1, "Error: Invalid redirect");
		const usize status = s_strtol10(dir.args[0].ptr, 3);
		location.redirectStatus = status;
		if (dir.args[0].length != 3 || status < 300 || status > 399 || !location.redirectStatus.is_valid())
			PERR_EXIT(1, "Error: Invalid redirect status");
		location.redirectTarget = beta.compress_span(locations, dir.args[1]);
		length = dir.args[1].length;
	}
	else
		PERR_EXIT(1, "Error: Invalid location directive");
	if (s_length_check(length))
		PERR_EXIT(1, "Error: Path size is too large");
}

PARSER_INL
(Span32) parse_cgi(const Array<Token> &tokens, usize &cursor, usize end, const Array<Location> &locations) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "cgi"))
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(1, "Error: Invalid CGI block");

	usize packSize = 0;
	cursor++;
	const usize definitionStart = cursor;
	while (cursor != end && tokens[cursor].type != Token::CLOSE_BRACKET) {
		const usize extensionIndex = cursor;
		const Span &extension = tokens[cursor].value;
		if (tokens[cursor].type != Token::WORD || extension.length < 2
			|| extension.ptr[0] != '.')
			PERR_EXIT(1, "Error: Invalid CGI extension");
		for (usize index = definitionStart; index < extensionIndex; index += 4) {
			const Span &previous = tokens[index].value;
			if (previous.length == extension.length && MEMCMP(previous.ptr, extension.ptr, extension.length) == 0)
				PERR_EXIT(1, "Error: Duplicate CGI extension");
		}
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::WORD
			|| !(tokens[cursor].value == "="))
			PERR_EXIT(1, "Error: Expected '=' in CGI definition");
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::WORD)
			PERR_EXIT(1, "Error: Invalid CGI interpreter");
		const Span &interpreter = tokens[cursor].value;
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::SEMICOLON)
			PERR_EXIT(1, "Error: Expected ';' after CGI definition");
		cursor++;
		packSize += sizeof(u16) * 2 + extension.length + interpreter.length;
	}
	if (cursor == end || tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Invalid CGI block");

	// Review
	const Span32 result = beta.compress_span(locations, packSize);
	Span packed = locations.extract(result);
	usize offset = 0;
	for (usize index = definitionStart; index < cursor; index += 4) {
		const Span &extension = tokens[index].value;
		const Span &interpreter = tokens[index + 2].value;
		const u16 lengths[2] = {(u16)extension.length, (u16)interpreter.length};
		MEMCPY_INLINE(packed.ptr + offset, lengths, sizeof(lengths));
		offset += sizeof(lengths);
		MEMCPY(packed.ptr + offset, extension.ptr, extension.length);
		offset += extension.length;
		MEMCPY(packed.ptr + offset, interpreter.ptr, interpreter.length);
		offset += interpreter.length;
	}
	return result;
}

PARSER_INL
(isize) parse_location(const Array<Token> &tokens, usize &cursor, usize end, Location &loc, const Array<Location> &locations) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "location"))
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(1, "Error: Expected location");
	const Span uri = tokens[cursor].value;
	if (uri.length == 0 || uri.ptr[0] != '/')
		PERR_EXIT(1, "Error: Invalid location path");
	if (s_length_check(uri.length))
		PERR_EXIT(1, "Error: Path size is too large");
	loc.uri = beta.compress_span(locations, uri);
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	const usize locationEnd = s_find_scope_end(tokens, cursor, end) + cursor;
	cursor++;
	bool cgiDefined = false;
	while (cursor != locationEnd) {
		if (tokens[cursor].type == Token::WORD && tokens[cursor].value == "cgi") {
			if (cgiDefined)
				PERR_EXIT(1, "Error: Duplicate CGI block");
			cgiDefined = true;
			loc.cgiBlock = parse_cgi(tokens, cursor, end, locations);
		}
		else {
			Directive dir = s_build_directive(alpha, tokens, cursor, end);
			parse_location_directive(loc, dir, locations);
		}
		cursor++;
	}
	if (tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	return 0;
}
