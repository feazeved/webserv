#pragma once
#include <sys/stat.h>
#include <unistd.h>

#include "core.hpp"
#include "HTTP.hpp"
#include "Parser.hpp"
#include "Parser_helpers.ipp"

namespace HTTP {
//

static inline
void s_set_methods(Array32<StringView32> &methods, Location &location) {
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

static inline 
void s_parse_location_directive(Location &location, const Array32<Token> &tokens, usize cursor, usize end) {
	Directive dir = s_build_directive(tokens, cursor, end);
	u32 length = 1;

	if (dir.name == "root") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid root");
		location.root = dir.args[0];
		length = dir.args[0].length;
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
		length = dir.args[0].length;
	}
	else if (dir.name == "upload_store") {
		struct stat st;
		if (dir.args.count != 1 || stat(dir.args[0].c_str(), &st) == -1
			|| !S_ISDIR(st.st_mode) || access(dir.args[0].c_str(), W_OK | X_OK) == -1)
			PERR_EXIT(1, "Error: Invalid upload store");
		location.uploadStore = dir.args[0];
		length = dir.args[0].length;
	}
	else if (dir.name == "return") {
		if (dir.args.count != 2)
			PERR_EXIT(1, "Error: Invalid redirect");
		usize status = s_strtol10(dir.args[0].c_str(), 3);
		location.redirectStatus = status;
		if (dir.args[0].length != 3 || status < 300 || status > 399
			|| !location.redirectStatus.is_valid())
			PERR_EXIT(1, "Error: Invalid redirect status");
		location.redirectTarget = dir.args[1];
		length = dir.args[1].length;
	}
	else
		PERR_EXIT(1, "Error: Invalid location directive");
	if (s_length_check(length))
		PERR_EXIT(1, "Error: Path size is too large");
}

static inline 
void s_parse_cgi(const Array32<Token> &tokens, usize &cursor, usize end, Location &loc) {
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
		const StringView32 &extension = tokens[cursor].value;
		if (tokens[cursor].type != Token::WORD || extension.length < 2
			|| extension.c_str()[0] != '.')
			PERR_EXIT(1, "Error: Invalid CGI extension");
		for (usize index = definitionStart; index < extensionIndex; index += 4) {
			const StringView32 &previous = tokens[index].value;
			if (previous.length == extension.length
				&& MEMCMP(previous.c_str(), extension.c_str(), extension.length) == 0)
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
		loc.cgiBlock = StringView32(tokens[cursor].value.offset - blockOffset + 1, blockOffset);
}

PARSER_INL
(isize) parse_location(const Array32<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "location"))
		PERR_EXIT(1, "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(1, "Error: Expected location");
	loc.url = tokens[cursor].value;
	if (loc.url.length == 0 || loc.url.c_str()[0] != '/')
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
			s_parse_cgi(tokens, cursor, locationEnd, loc);
		}
		else
			s_parse_location_directive(loc, tokens, cursor, end);
		cursor++;
	}
	if (tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(1, "Error: Unexpected token");
	return 0;
}

//
}	// Namespace HTTP
