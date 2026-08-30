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
(Span32) parse_cgi(Array<Token> &tokArray, const Array<Location> &locations) {
	if (tokArray[0].type != Token::OPEN_BRACKET)
		PERR_EXIT(1, "Error: Invalid CGI block");

	usize packSize = 0;
	tokArray.ptr++;
	Token *definitionStart = tokArray.ptr;
	while (tokArray[0].type != Token::CLOSE_BRACKET) {
		Token *definition = tokArray.ptr;
		const Span &extension = tokArray[0].value;
		if (extension.length < 2 || extension.ptr[0] != '.')
			PERR_EXIT(1, "Error: Invalid CGI extension");
		for (Token *previousToken = definitionStart; previousToken < definition; previousToken += 4) {
			const Span &previous = previousToken->value;
			if (previous.length == extension.length && MEMCMP(previous.ptr, extension.ptr, extension.length) == 0)
				PERR_EXIT(1, "Error: Duplicate CGI extension");
		}
		tokArray.ptr++;
		if (!(tokArray[0].value == "="))
			PERR_EXIT(1, "Error: Expected '=' in CGI definition");
		tokArray.ptr++;
		if (tokArray[0].type != Token::WORD)
			PERR_EXIT(1, "Error: Invalid CGI interpreter");
		const Span &interpreter = tokArray[0].value;
		tokArray.ptr++;
		if (tokArray[0].type != Token::SEMICOLON)
			PERR_EXIT(1, "Error: Expected ';' after CGI definition");
		tokArray.ptr++;
		packSize += sizeof(u16) * 2 + extension.length + interpreter.length;
	}

	const Span32 result = beta.compress_span(locations, packSize);
	Span packed = locations.extract(result);
	usize offset = 0;
	for (Token *definition = definitionStart; definition < tokArray.ptr; definition += 4) {
		const Span &extension = definition[0].value;
		const Span &interpreter = definition[2].value;
		const u16 lengths[2] = {(u16)extension.length, (u16)interpreter.length};
		MEMCPY_INLINE(packed.ptr + offset, lengths, sizeof(lengths));
		offset += sizeof(lengths);
		MEMCPY(packed.ptr + offset, extension.ptr, extension.length);
		offset += extension.length;
		MEMCPY(packed.ptr + offset, interpreter.ptr, interpreter.length);
		offset += interpreter.length;
	}
	tokArray.ptr++;
	return result;
}

PARSER_INL
(void) parse_location(Array<Token> &tokArray, Location &loc, const Array<Location> &locations) {
	const Span uri = tokArray[0].value;
	if (uri.length == 0 || uri.ptr[0] != '/')
		PERR_EXIT(1, "Error: Invalid location path");
	if (s_length_check(uri.length))
		PERR_EXIT(1, "Error: Path size is too large");
	loc.uri = beta.compress_span(locations, uri);
	tokArray.ptr += 2;
	bool cgiDefined = false;
	while (tokArray[0].type != Token::CLOSE_BRACKET) {
		if (tokArray[0].value == "cgi") {
			if (cgiDefined)
				PERR_EXIT(1, "Error: Duplicate CGI block");
			cgiDefined = true;
			tokArray.ptr++;
			loc.cgiBlock = parse_cgi(tokArray, locations);
		}
		else {
			Directive dir = s_build_directive(alpha, tokArray);
			parse_location_directive(loc, dir, locations);
		}
	}
	tokArray.ptr++;
}
