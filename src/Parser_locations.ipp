#pragma once
#include "core.hpp"
#include "HTTP.hpp"
#include "Parser.hpp"
#include "Parser_helpers.ipp"

namespace HTTP {
//

static inline
void s_set_methods(Array32<StringView> &methods, Location &location) {
	for (u32 index = 0; index < methods.count; index++)
	{
		if (methods[index] == "GET")
			location.methods |= Mode::GET;
		else if (methods[index] == "POST")
			location.methods |= Mode::POST;
		else if (methods[index] == "DELETE")
			location.methods |= Mode::DELETE;
		else
			PERR_EXIT(1, "Error: Invalid method");
	}
}

PARSER_INL
(isize) set_location_directive(Directive &dir, Location &location) {
	if (dir.name == "root") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid root");
		location.root = dir.args[0];
	}
	else if (dir.name == "autoindex") {
		if (dir.args.count != 1 || (!(dir.args[0] == "on") && !(dir.args[0] == "off")))
			PERR_EXIT(1, "Error: Invalid autoindex");
		location.autoindex = dir.args[0] == "on" ? true : false;
	}
	else if (dir.name == "allowed_methods") {
		if (dir.args.count == 0)
			PERR_EXIT(1, "Error: No allowed methods defined");
		s_set_methods(dir.args, location);
	}
	else if (dir.name == "index") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid index");
		location.index = dir.args[0];
	}
	else if (dir.name == "upload_store") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid upload store");
		location.uploadStore = dir.args[0];
	}
	else if (dir.name == "return") {
		if (dir.args.count != 2)
			PERR_EXIT(1, "Error: Invalid redirect");
		usize status = s_strtol10(dir.args[0].get(), 3);
		location.redirectStatus = status;
		if (dir.args[0].length != 3 || status < 300 || status > 399
			|| !location.redirectStatus.is_valid())
			PERR_EXIT(1, "Error: Invalid redirect status");
		location.redirectTarget = dir.args[1];
	}
	else
		PERR_EXIT(1, "Error: Invalid location directive");
	return 0;
}

PARSER_INL
(isize) parse_cgi(const Array32<Token> &tokens, usize &cursor, usize end, Location &loc) {
	if (cursor == end || tokens[cursor].type != Token::WORD
		|| !(tokens[cursor].value == "cgi"))
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(1, "Error: Invalid CGI block");

	const u32 blockOffset = tokens[cursor].value.offset;
	cursor++;
	const usize definitionStart = cursor;
	while (cursor != end && tokens[cursor].type != Token::CLOSE_BRACKET) {
		const usize extensionIndex = cursor;
		const StringView &extension = tokens[cursor].value;
		if (tokens[cursor].type != Token::WORD || extension.length < 2
			|| extension.get()[0] != '.')
			PERR_EXIT(1, "Error: Invalid CGI extension");
		for (usize index = definitionStart; index < extensionIndex; index += 4) {
			const StringView &previous = tokens[index].value;
			if (previous.length == extension.length
				&& MEMCMP(previous.get(), extension.get(), extension.length) == 0)
				PERR_EXIT(1, "Error: Duplicate CGI extension");
		}
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::WORD
			|| !(tokens[cursor].value == "="))
			PERR_EXIT(1, "Error: Expected '=' in CGI definition");
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::WORD)
			PERR_EXIT(1, "Error: Invalid CGI interpreter");
		cursor++;
		if (cursor == end || tokens[cursor].type != Token::SEMICOLON)
			PERR_EXIT(1, "Error: Expected ';' after CGI definition");
		cursor++;
	}
	if (cursor == end || tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Invalid CGI block");
	if (cursor != definitionStart)
		loc.cgiBlock = StringView(tokens[cursor].value.offset - blockOffset + 1, blockOffset);
	return 0;
}

PARSER_INL
(isize) parse_location(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "location"))
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(1, "Error: Expected location");
	loc.url = tokens[cursor].value;
	if (loc.url.length == 0 || loc.url.get()[0] != '/')
		PERR_EXIT(1, "Error: Invalid location path");
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
			parse_cgi(tokens, cursor, locationEnd, loc);
		}
		else {
			Directive dir;
			parse_directive(tokens, cursor, locationEnd, dir);
			set_location_directive(dir, loc);
		}
		cursor++;
	}
	if (tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	return 0;
}

//
}	// Namespace HTTP
