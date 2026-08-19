#pragma once

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <sstream>
#include "HTTP.hpp"
#include "parse_config_helpers.ipp"

namespace HTTP {
namespace Parse {
//



isize parse_directive(tokIter &cursor, tokIter &end, Directive &dir) {
	std::vector<std::string> arguments;

	dir.name = cursor->value;
	cursor++;
	while (cursor != end && cursor->type != Token::SEMICOLON) {
		arguments.push_back(cursor->value);
		s_advance(cursor, end);
	}
	if (cursor->type != Token::SEMICOLON)
		throw std::runtime_error("Unexpected token");
	dir.args = arguments;
	return 0;
}

isize set_location_directive(Directive &dir, HTTP::Location &location) {
	if (dir.name == "root") {
		if (dir.args.size() != 1)
			throw std::runtime_error("Invalid root");
		location.root = dir.args.at(0);
	}
	else if (dir.name == "autoindex") {
		if (dir.args.size() != 1 || (dir.args.at(0) != "on" && dir.args.at(0) != "off"))
			throw std::runtime_error("Invalid autoindex");
		location.autoindex = dir.args.at(0) == "on" ? true : false;
	}
	else if (dir.name == "allowed_methods") {
		if (dir.args.size() == 0)
			throw std::runtime_error("No allowed methods defined");
		s_set_methods(dir.args, location);
	}
	else if (dir.name == "index") {
		if (dir.args.size() != 1)
			throw std::runtime_error("Invalid index");
		location.index = dir.args.at(0);
	}
	else if (dir.name == "upload_store") {
		if (dir.args.size() != 1)
			throw std::runtime_error("Invalid upload store");
		location.upload_store = dir.args.at(0);
	}
	else if (dir.name == "return") {
		if (dir.args.size() != 2)
			throw std::runtime_error("Invalid redirect");
		HTTP_STATUS(300);
		location.redirect = HTTP::Status::i300;
	}
	else
		throw std::runtime_error("Invalid location directive");
	return 0;
}

isize parse_location(tokIter &cursor, tokIter &end, HTTP::Location &loc) {
	s_match(cursor, "location");
	if (cursor->type != Token::WORD)
		throw std::runtime_error("Expected location");
	loc.path = cursor->value;
	cursor++;
	s_match(cursor, "{");
	while (cursor != end && cursor->type != Token::CLOSE_BRACKET) {
		Directive dir;
		parse_directive(cursor, end, dir);
		set_location_directive(dir, loc);
		cursor++;
	}
	if (cursor->type != Token::CLOSE_BRACKET)
		throw std::runtime_error("Unexpected token");
	return 0;
}

isize set_server_directive(Directive &dir, HTTP::ServerConfig &server) {
	if (dir.name == "listen") {
		if (server.port != -1 || dir.args.size() != 1)
			throw std::runtime_error("Invalid port definition");
		server.port = s_strtol(dir.args.at(0));
		if (server.port < 1 || server.port > 65535)
			throw std::runtime_error("Invalid port");
	}
	else if (dir.name == "host") {
		if (server.host != "localhost" || dir.args.size() != 1)
			throw std::runtime_error("Invalid host definition");
		server.host = dir.args.at(0);
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.size() != 1)
			throw std::runtime_error("Invalid max body size definition");
		server.maxBodySize = s_strtol(dir.args.at(0));
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			throw std::runtime_error("Invalid max body size");
	}
	// TODO: Auto index is only used in locations
	// else if (dir.name == "autoindex") {
	// 	if (dir.args.size() != 1 || (dir.args.at(0) != "on" && dir.args.at(0) != "off"))
	// 		throw std::runtime_error("Invalid autoindex");
	// 	server.autoindex = dir.args.at(0) == "on" ? true : false;
	// }	// TODO: this will be different
	else if (dir.name == "error_page") {
		if (dir.args.size() != 2)
			throw std::runtime_error("Invalid error page");
		i64	error = s_strtol(dir.args.at(0));
		if (error < 400 || error > 599)
			throw std::runtime_error("Invalid error number");
		server.errors[error] = dir.args.at(1);
	}
	else
		throw std::runtime_error("Invalid server directive");
	return 0;
}

isize parse_server(tokIter cursor, tokIter end, HTTP::ServerConfig &server) {
	s_match(cursor, "server");
	s_match(cursor, "{");
	if (cursor == end || cursor->value == "}")
		throw std::runtime_error("Empty server block");
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
		s_advance(cursor, end);
	}
	if (end->type != Token::CLOSE_BRACKET)
		throw std::runtime_error("Unexpected token");
	return 0;
}

isize parse_config(char *filePath, std::vector<ServerConfig> &serverConfigs) {
	std::stringstream stream;
	std::ifstream inputFile(filePath);

	inputFile.close();

	std::vector<Token> tokens;
	if (tokenizer(stream, tokens) < 0)
		return -1;
	tokIter it = tokens.begin();
	tokIter end = tokens.end();

	//tokenizerDump(tokens);

	while (it != end) {
		if (it->value == "server") {
			usize distance = s_find_scope_end(it, end);
			HTTP::ServerConfig serverConf;
			parse_server(it, it + distance, serverConf);
			serverConfigs.push_back(serverConf);
			it = it + distance;
		}
		else
			throw std::runtime_error("Unexpected token");
		it++;
	}

	//configDump(ret);
	return 0;
}

//
}	// Namespace Parse
}	// Namespace HTTP