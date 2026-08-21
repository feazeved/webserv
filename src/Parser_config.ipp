#pragma once
#include <vector>
#include "core.hpp"
#include "HTTP.hpp"
#include "Parser.hpp"

namespace HTTP {
//

PARSER_INL
(usize) find_scope_end(tokIter &begin, tokIter &end) {
	tokIter it = begin;
	bool startedCount = false;
	int braces = 0;
	usize distance = 0;

	while (it != end) {
		if (it->type == Token::OPEN_BRACKET) {
			startedCount = true;
			braces++;
		}
		else if (it->type == Token::CLOSE_BRACKET) {
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
(isize) parse_directive(tokIter &cursor, tokIter &end, Directive &dir) {
	std::vector<StringView> arguments;

	if (cursor == end || cursor->type != Token::WORD)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	dir.name = cursor->value;
	cursor++;
	while (cursor != end && cursor->type == Token::WORD) {
		arguments.push_back(cursor->value);
		cursor++;
	}
	if (cursor == end || cursor->type != Token::SEMICOLON)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	dir.args = arguments;
	return 0;
}

PARSER_INL
(isize) set_location_directive(Directive &dir, HTTP::Location &location) {
	if (dir.name == "root") {
		if (dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid root");
		location.root = dir.args.at(0);
	}
	else if (dir.name == "autoindex") {
		if (dir.args.size() != 1 || (!(dir.args.at(0) == "on") && !(dir.args.at(0) == "off")))
			PERR_EXIT(cleanup(), "Error: Invalid autoindex");
		location.autoindex = dir.args.at(0) == "on" ? true : false;
	}
	else if (dir.name == "allowed_methods") {
		if (dir.args.size() == 0)
			PERR_EXIT(cleanup(), "Error: No allowed methods defined");
		set_methods(dir.args, location);
	}
	else if (dir.name == "index") {
		if (dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid index");
		location.index = dir.args.at(0);
	}
	else if (dir.name == "upload_store") {
		if (dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid upload store");
		location.uploadStore = dir.args.at(0);
	}
	else if (dir.name == "cgi") {
		if (dir.args.size() != 2 || dir.args.at(0).length < 2
			|| dir.args.at(0).get()[0] != '.' || location.cgiExtension.length != 0)
			PERR_EXIT(cleanup(), "Error: Invalid CGI definition");
		location.cgiExtension = dir.args.at(0);
		location.cgiInterpreter = dir.args.at(1);
	}
	else if (dir.name == "return") {
		if (dir.args.size() != 2)
			PERR_EXIT(cleanup(), "Error: Invalid redirect");
		usize status = s_strtol10(dir.args.at(0).get(), 3);
		location.redirectStatus = status;
		if (dir.args.at(0).length != 3 || status < 300 || status > 399
			|| !location.redirectStatus.is_valid())
			PERR_EXIT(cleanup(), "Error: Invalid redirect status");
		location.redirectTarget = dir.args.at(1);
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid location directive");
	return 0;
}

PARSER_INL
(void) set_methods(std::vector<StringView> &methods, HTTP::Location &location) {
	std::vector<StringView>::iterator it = methods.begin();

	for (; it != methods.end(); it++)
	{
		if (*it == "GET")
			location.methods |= HTTP::Mode::GET;
		else if (*it == "POST")
			location.methods |= HTTP::Mode::POST;
		else if (*it == "DELETE")
			location.methods |= HTTP::Mode::DELETE;
		else
			PERR_EXIT(cleanup(), "Error: Invalid method");
	}
}

PARSER_INL
(isize) parse_location(tokIter &cursor, tokIter &end, HTTP::Location &loc) {
	if (cursor == end || cursor->type != Token::WORD || !(cursor->value == "location"))
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || cursor->type != Token::WORD)
		PERR_EXIT(cleanup(), "Error: Expected location");
	loc.path = cursor->value;
	cursor++;
	if (cursor == end || cursor->type != Token::OPEN_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	while (cursor != end && cursor->type != Token::CLOSE_BRACKET) {
		Directive dir;
		parse_directive(cursor, end, dir);
		set_location_directive(dir, loc);
		cursor++;
	}
	if (cursor == end || cursor->type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	return 0;
}

PARSER_INL
(isize) set_server_directive(Directive &dir, HTTP::ServerConfig &server) {
	if (dir.name == "listen") {
		if (server.port != SIZE_MAX || dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid port definition");
		server.port = s_strtol10(dir.args.at(0).get(), dir.args.at(0).length);
		if (server.port < 1 || server.port > 65535)
			PERR_EXIT(cleanup(), "Error: Invalid port");
	}
	else if (dir.name == "host") {
		if (server.host.length != 0 || dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid host definition");
		server.host = dir.args.at(0);
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid max body size definition");
		server.maxBodySize = s_strtol10(dir.args.at(0).get(), dir.args.at(0).length);
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			PERR_EXIT(cleanup(), "Error: Invalid max body size");
		server.maxBodySize <<= 20;
	}
	else if (dir.name == "error_page") {
		if (dir.args.size() != 2)
			PERR_EXIT(cleanup(), "Error: Invalid error page");
		usize error = s_strtol10(dir.args.at(0).get(), dir.args.at(0).length);
		if (error < 400 || error > 599)
			PERR_EXIT(cleanup(), "Error: Invalid error number");
		// server.errors[error] = dir.args.at(1);	// TODO
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid server directive");
	return 0;
}

PARSER_INL
(isize) parse_server(tokIter cursor, tokIter end, HTTP::ServerConfig &server) {
	if (cursor == end || cursor->type != Token::WORD || !(cursor->value == "server"))
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || cursor->type != Token::OPEN_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
	if (cursor == end || cursor->type == Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Empty server block");
	while (cursor != end) {
		if (cursor->value == "location") {	// TODO: can they be case insenstive?
			HTTP::Location loc;
			parse_location(cursor, end, loc);
			server.locations.push_back(loc);
		}
		else {
			Directive dir;
			parse_directive(cursor, end, dir);
			set_server_directive(dir, server);
		}
		cursor++;
	}
	if (end->type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	if (server.host.length == 0)
		server.host = StringView(sizeof("localhost") - 1, (u32)fileSize + 4);
	return 0;
}

PARSER_INL
(std::vector<ServerConfig>) parse_config() {
	usize serverCount = s_count_servers(fileBuffer, fileSize);
	if (serverCount > MAX_VIRTUAL_SERVERS)
		PERR_EXIT(cleanup(), "Error: Invalid config");

	std::vector<Token> tokVector = tokenize();
	tokIter it = tokVector.begin();
	tokIter end = tokVector.end();

	std::vector<ServerConfig> cfgVector;
	cfgVector.reserve(serverCount);

	while (it != end) {
		if (it->value == "server") {
			usize distance = find_scope_end(it, end);
			if (distance == SIZE_MAX)
				PERR_EXIT(cleanup(), "Error: Invalid server block");
			HTTP::ServerConfig serverConf;
			parse_server(it, it + distance, serverConf);
			cfgVector.push_back(serverConf);
			it = it + distance;
		}
		else
			PERR_EXIT(cleanup(), "Error: Unexpected token");
		it++;
	}

	return cfgVector;
}

//
}	// Namespace HTTP
