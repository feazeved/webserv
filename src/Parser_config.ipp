#pragma once
#include <string>
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
	return distance;
}

PARSER_INL
(void) match(tokIter &cursor, const std::string &value) {
	if (cursor->value != value)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	cursor++;
}

PARSER_INL
(void) advance(tokIter &cursor, tokIter &end) {
	if (cursor == end)
		PERR_EXIT(cleanup(), "Error: Invalid read");
	cursor++;
}

PARSER_INL
(isize) parse_directive(tokIter &cursor, tokIter &end, Directive &dir) {
	std::vector<std::string> arguments;

	dir.name = cursor->value;
	cursor++;
	while (cursor != end && cursor->type != Token::SEMICOLON) {
		arguments.push_back(cursor->value);
		advance(cursor, end);
	}
	if (cursor->type != Token::SEMICOLON)
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
		if (dir.args.size() != 1 || (dir.args.at(0) != "on" && dir.args.at(0) != "off"))
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
		location.upload_store = dir.args.at(0);
	}
	else if (dir.name == "return") {
		if (dir.args.size() != 2)
			PERR_EXIT(cleanup(), "Error: Invalid redirect");
		HTTP_STATUS(300);
		location.redirect = HTTP::Status::i300;
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid location directive");
	return 0;
}

PARSER_INL
(void) set_methods(std::vector<std::string> &methods, HTTP::Location &location) {
	std::vector<std::string>::iterator it = methods.begin();

	for (; it != methods.end(); it++)
	{
		if (MEMCMP_INLINE(it->c_str(), "GET") == 0)
			location.methods |= HTTP::Mode::GET;
		else if (MEMCMP_INLINE(it->c_str(), "POST") == 0)
			location.methods |= HTTP::Mode::POST;
		else if (MEMCMP_INLINE(it->c_str(), "DELETE") == 0)
			location.methods |= HTTP::Mode::DELETE;
		else
			PERR_EXIT(cleanup(), "Error: Invalid method");
	}
}

PARSER_INL
(isize) parse_location(tokIter &cursor, tokIter &end, HTTP::Location &loc) {
	match(cursor, "location");
	if (cursor->type != Token::WORD)
		PERR_EXIT(cleanup(), "Error: Expected location");
	loc.path = cursor->value;
	cursor++;
	match(cursor, "{");
	while (cursor != end && cursor->type != Token::CLOSE_BRACKET) {
		Directive dir;
		parse_directive(cursor, end, dir);
		set_location_directive(dir, loc);
		cursor++;
	}
	if (cursor->type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	return 0;
}

PARSER_INL
(isize) set_server_directive(Directive &dir, HTTP::ServerConfig &server) {
	if (dir.name == "listen") {
		if (server.port != SIZE_MAX || dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid port definition");
		server.port = s_strtol10(dir.args.at(0).c_str());
		if (server.port < 1 || server.port > 65535)
			PERR_EXIT(cleanup(), "Error: Invalid port");
	}
	else if (dir.name == "host") {
		if (server.host != "localhost" || dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid host definition");
		server.host = dir.args.at(0);
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.size() != 1)
			PERR_EXIT(cleanup(), "Error: Invalid max body size definition");
		server.maxBodySize = s_strtol10(dir.args.at(0).c_str());
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			PERR_EXIT(cleanup(), "Error: Invalid max body size");
		server.maxBodySize <<= 20;
	}
	else if (dir.name == "error_page") {
		if (dir.args.size() != 2)
			PERR_EXIT(cleanup(), "Error: Invalid error page");
		usize error = s_strtol10(dir.args.at(0).c_str());
		if (error < 400 || error > 599)
			PERR_EXIT(cleanup(), "Error: Invalid error number");
		server.errors[error] = dir.args.at(1);
	}
	else
		PERR_EXIT(cleanup(), "Error: Invalid server directive");
	return 0;
}

PARSER_INL
(isize) parse_server(tokIter cursor, tokIter end, HTTP::ServerConfig &server) {
	match(cursor, "server");
	match(cursor, "{");
	if (cursor == end || cursor->value == "}")
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
		advance(cursor, end);
	}
	if (end->type != Token::CLOSE_BRACKET)
		PERR_EXIT(cleanup(), "Error: Unexpected token");
	return 0;
}

PARSER_INL
(std::vector<ServerConfig>) parse_config(char *fileBuffer, usize totalBytes) {
	std::vector<Token> tokVector = tokenizer(fileBuffer);
	tokIter it = tokVector.begin();
	tokIter end = tokVector.end();

	std::vector<ServerConfig> cfgVector;
	usize serverCount = s_count_servers(fileBuffer, totalBytes);
	if (serverCount == SIZE_MAX)
		PERR_EXIT(1, "Error: Invalid config");
	cfgVector.reserve(serverCount);

	while (it != end) {
		if (it->value == "server") {
			usize distance = find_scope_end(it, end);
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