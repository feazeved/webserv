#pragma once
#include "core.hpp"
#include "HTTP.hpp"
#include "Parser.hpp"

namespace HTTP {
//

PARSER_INL
(usize) find_scope_end(const Array<Token> &tokens, usize begin, usize end) {
	usize it = begin;
	bool startedCount = false;
	int braces = 0;
	usize distance = 0;

	while (it != end) {
		if (tokens[it].type == Token::OPEN_BRACKET) {
			startedCount = true;
			braces++;
		}
		else if (tokens[it].type == Token::CLOSE_BRACKET) {
			startedCount = true;
			braces--;
		}
		if (!braces && startedCount)
			break;
		it++;
		distance++;
	}
	return it == end ? SIZE_MAX : distance;
}

PARSER_INL
(usize) count_locations(const Array<Token> &tokens, usize cursor, usize end) {
	usize locationCount = 0;

	while (cursor < end) {
		if (tokens[cursor].value == "location") {
			usize distance = find_scope_end(tokens, cursor, end);
			if (distance == SIZE_MAX)
				PERR_EXIT(cleanup(), "Error: Invalid location block");
			locationCount++;
			cursor += distance + 1;
		}
		else {
			while (cursor < end && tokens[cursor].type != Token::SEMICOLON)
				cursor++;
			if (cursor < end)
				cursor++;
		}
	}
	return locationCount;
}

PARSER_INL
(isize) parse_directive(const Array<Token> &tokens, usize &cursor, usize end, Directive &dir) {
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	dir.name = tokens[cursor].value;
	cursor++;
	usize argumentStart = cursor;
	while (cursor != end && tokens[cursor].type == Token::WORD)
		cursor++;
	if (cursor == end || tokens[cursor].type != Token::SEMICOLON)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	if (dir.args.alloc((u32)(cursor - argumentStart)) == true)
		_exit(1);
	for (u32 index = 0; index < dir.args.count; index++)
		dir.args[index] = tokens[argumentStart + index].value;
	return 0;
}

PARSER_INL
(isize) set_location_directive(Directive &dir, HTTP::Location &location) {
	if (dir.name == "root") {
		if (dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid root");
		location.root = dir.args[0];
	}
	else if (dir.name == "autoindex") {
		if (dir.args.count != 1 || (!(dir.args[0] == "on") && !(dir.args[0] == "off")))
			PERR_EXIT(cleanup(), "Error: Invalid autoindex");
		location.autoindex = dir.args[0] == "on" ? true : false;
	}
	else if (dir.name == "allowed_methods") {
		if (dir.args.count == 0)
			PERR_EXIT(cleanup(), "Error: No allowed methods defined");
		set_methods(dir.args, location);
	}
	else if (dir.name == "index") {
		if (dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid index");
		location.index = dir.args[0];
	}
	else if (dir.name == "upload_store") {
		if (dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid upload store");
		location.uploadStore = dir.args[0];
	}
	else if (dir.name == "cgi") {
		if (dir.args.count != 2 || dir.args[0].length < 2
			|| dir.args[0].get()[0] != '.' || location.cgiExtension.length != 0)
			PERR_EXIT(cleanup(), "Error: Invalid CGI definition");
		location.cgiExtension = dir.args[0];
		location.cgiInterpreter = dir.args[1];
	}
	else if (dir.name == "return") {
		if (dir.args.count != 2)
			PERR_EXIT(cleanup(), "Error: Invalid redirect");
		usize status = s_strtol10(dir.args[0].get(), 3);
		location.redirectStatus = status;
		if (dir.args[0].length != 3 || status < 300 || status > 399
			|| !location.redirectStatus.is_valid())
			PERR_EXIT(cleanup(), "Error: Invalid redirect status");
		location.redirectTarget = dir.args[1];
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid location directive");
	return 0;
}

PARSER_INL
(void) set_methods(Array<StringView> &methods, HTTP::Location &location) {
	for (u32 index = 0; index < methods.count; index++)
	{
		if (methods[index] == "GET")
			location.methods |= HTTP::Mode::GET;
		else if (methods[index] == "POST")
			location.methods |= HTTP::Mode::POST;
		else if (methods[index] == "DELETE")
			location.methods |= HTTP::Mode::DELETE;
		else
			PERR_EXIT(cleanup(), "Error: Invalid method");
	}
}

PARSER_INL
(isize) parse_location(const Array<Token> &tokens, usize &cursor, usize end, HTTP::Location &loc) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "location"))
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::WORD)
		PERR_EXIT(cleanup(), "Error: Expected location");
	loc.path = tokens[cursor].value;
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	while (cursor != end && tokens[cursor].type != Token::CLOSE_BRACKET) {
		Directive dir;
		parse_directive(tokens, cursor, end, dir);
		set_location_directive(dir, loc);
		cursor++;
	}
	if (cursor == end || tokens[cursor].type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	return 0;
}

PARSER_INL
(isize) set_server_directive(Directive &dir, HTTP::ServerConfig &server) {
	if (dir.name == "listen") {
		if (server.port != SIZE_MAX || dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid port definition");
		server.port = s_strtol10(dir.args[0].get(), dir.args[0].length);
		if (server.port < 1 || server.port > 65535)
			PERR_EXIT(cleanup(), "Error: Invalid port");
	}
	else if (dir.name == "host") {
		if (server.host.length != 0 || dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid host definition");
		server.host = dir.args[0];
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.count != 1)
			PERR_EXIT(cleanup(), "Error: Invalid max body size definition");
		server.maxBodySize = s_strtol10(dir.args[0].get(), dir.args[0].length);
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			PERR_EXIT(cleanup(), "Error: Invalid max body size");
		server.maxBodySize <<= 20;
	}
	else if (dir.name == "error_page") {
		if (dir.args.count != 2)
			PERR_EXIT(cleanup(), "Error: Invalid error page");
		usize error = s_strtol10(dir.args[0].get(), dir.args[0].length);
		if (error < 400 || error > 599)
			PERR_EXIT(cleanup(), "Error: Invalid error number");
		// server.errors[error] = dir.args[1];	// TODO
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid server directive");
	return 0;
}

PARSER_INL
(isize) parse_server(const Array<Token> &tokens, usize cursor, usize end, HTTP::ServerConfig &server) {
	if (cursor == end || tokens[cursor].type != Token::WORD || !(tokens[cursor].value == "server"))
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type != Token::OPEN_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || tokens[cursor].type == Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Empty server block");

	usize locationCount = count_locations(tokens, cursor, end);
	if (server.locations.alloc(locationCount) == true)
		_exit(1);

	usize locationIndex = 0;
	while (cursor != end) {
		if (tokens[cursor].value == "location") {	// TODO: can they be case insenstive?
			HTTP::Location loc;
			parse_location(tokens, cursor, end, loc);
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
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	if (server.host.length == 0)
		server.host = StringView(sizeof("localhost") - 1, (u32)fileSize + 4);
	return 0;
}

PARSER_INL
(void) parse_config(ServerConfig (&servers)[MAX_VIRTUAL_SERVERS]) {

	Array<Token> tokArray = tokenize();
	usize cursor = 0;
	usize end = tokArray.count;
	usize serverIndex = 0;

	while (cursor != end) {
		if (tokArray[cursor].value == "server") {
			usize distance = find_scope_end(tokArray, cursor, end);
			if (distance == SIZE_MAX)
				PERR_EXIT(cleanup(), "Error: Invalid server block");
			HTTP::ServerConfig serverConf;
			parse_server(tokArray, cursor, cursor + distance, serverConf);
			servers[serverIndex] = serverConf;
			serverIndex++;
			cursor += distance;
		}
		else
			PERR_EXIT(cleanup(), "Error: Unexpected token");
		cursor++;
	}
}

//
}	// Namespace HTTP
