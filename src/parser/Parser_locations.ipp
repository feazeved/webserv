#pragma once
#include "Parser.hpp"

static inline
void s_set_methods(const ArrayView<Span> &methods, Parser::ParsedLocation &location) {
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
(void) parse_location_directive(ParsedLocation &location, Directive &dir) {
	usize length = 1;

	if (dir.name == "root") {
		if (dir.args.count != 1)
			PERR_EXIT(1, "Error: Invalid root");
		location.root = dir.args[0];
		length = dir.args[0].size;
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
		location.index = dir.args[0];
		length = dir.args[0].size;
	}
	else if (dir.name == "upload_store") {
		struct stat st;
		if (dir.args.count != 1 || stat(dir.args[0].ptr, &st) == -1
			|| !S_ISDIR(st.st_mode) || access(dir.args[0].ptr, W_OK | X_OK) == -1)
			PERR_EXIT(1, "Error: Invalid upload store");
		location.uploadStore = dir.args[0];
		length = dir.args[0].size;
	}
	else if (dir.name == "return") {
		if (dir.args.count != 2)
			PERR_EXIT(1, "Error: Invalid redirect");
		const usize status = s_strtol10(dir.args[0].ptr, 3);
		location.redirectStatus = status;
		if (dir.args[0].size != 3 || status < 300 || status > 399 || !location.redirectStatus.is_valid())
			PERR_EXIT(1, "Error: Invalid redirect status");
		location.redirectTarget = dir.args[1];
		length = dir.args[1].size;
	}
	else
		PERR_EXIT(1, "Error: Invalid location directive");
	if (s_length_check(length))
		PERR_EXIT(1, "Error: Path size is too large");
}

PARSER_INL
(Parser::ParsedCgi) parse_cgi(ArrayView<Token> &tokArray) {
	if (tokArray[0].type != Token::OPEN_BRACKET)
		PERR_EXIT(1, "Error: Invalid CGI block");

	ParsedCgi cgi;
	tokArray.ptr++;
	Token *definitionStart = tokArray.ptr;
	while (tokArray[0].type != Token::CLOSE_BRACKET) {
		Token *definition = tokArray.ptr;
		const Span &extension = tokArray[0].value;
		if (extension.size < 2 || extension.ptr[0] != '.')
			PERR_EXIT(1, "Error: Invalid CGI extension");
		for (Token *previousToken = definitionStart; previousToken < definition; previousToken += 4) {
			const Span &previous = previousToken->value;
			if (previous.size == extension.size && MEMCMP(previous.ptr, extension.ptr, extension.size) == 0)
				PERR_EXIT(1, "Error: Duplicate CGI extension");
		}
		tokArray.ptr++;
		if (!(tokArray[0].value == "="))
			PERR_EXIT(1, "Error: Expected '=' in CGI definition");
		tokArray.ptr++;
		if (tokArray[0].type != Token::WORD)
			PERR_EXIT(1, "Error: Invalid CGI interpreter");
		const Span &interpreter = tokArray[0].value;
		if (access(interpreter.ptr, X_OK) == -1)	// TODO: Check if more is not needed
			PERR_EXIT(1, "Error: Invalid CGI interpreter");
		tokArray.ptr++;
		if (tokArray[0].type != Token::SEMICOLON)
			PERR_EXIT(1, "Error: Expected ';' after CGI definition");
		tokArray.ptr++;
		cgi.size += sizeof(u16) * 2 + extension.size + interpreter.size;
	}

	cgi.definitions = ArrayView<Token>(definitionStart, (usize)(tokArray.ptr - definitionStart));
	tokArray.ptr++;
	return cgi;
}

PARSER_INL
(Parser::ParsedLocation) parse_location(ArrayView<Token> &tokArray) {
	ParsedLocation loc;
	loc.uri = tokArray[0].value;

	if (loc.uri.size == 0 || loc.uri.ptr[0] != '/')
		PERR_EXIT(1, "Error: Invalid location path");
	if (s_length_check(loc.uri.size))
		PERR_EXIT(1, "Error: Path size is too large");
	tokArray.ptr += 2;
	bool cgiDefined = false;
	while (tokArray[0].type != Token::CLOSE_BRACKET) {
		if (tokArray[0].value == "cgi") {
			if (cgiDefined)
				PERR_EXIT(1, "Error: Duplicate CGI block");
			cgiDefined = true;
			tokArray.ptr++;
			loc.cgiBlock = parse_cgi(tokArray);
		}
		else {
			Directive dir = s_build_directive(alpha, tokArray);
			parse_location_directive(loc, dir);
		}
	}
	tokArray.ptr++;
	return loc;
}
