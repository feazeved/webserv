#pragma once
#include <string>
#include <vector>

#include "core.hpp"
#include "HTTP.hpp"
#include "parse_tokens.ipp"
#include "parse_helpers.ipp"

namespace HTTP {
namespace Parse {
//

static inline
isize s_parse_directive(tokIter &cursor, tokIter &end, Directive &dir) {
	std::vector<std::string> arguments;

	dir.name = cursor->value;
	cursor++;
	while (cursor != end && cursor->type != Token::SEMICOLON) {
		arguments.push_back(cursor->value);
		s_advance(cursor, end);
	}
	if (cursor->type != Token::SEMICOLON)
		PERR_EXIT(s_cleanup(NULL), "Error: Unexpected token");
	dir.args = arguments;
	return 0;
}

static inline
isize s_set_location_directive(Directive &dir, HTTP::Location &location) {
	if (dir.name == "root") {
		if (dir.args.size() != 1)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid root");
		location.root = dir.args.at(0);
	}
	else if (dir.name == "autoindex") {
		if (dir.args.size() != 1 || (dir.args.at(0) != "on" && dir.args.at(0) != "off"))
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid autoindex");
		location.autoindex = dir.args.at(0) == "on" ? true : false;
	}
	else if (dir.name == "allowed_methods") {
		if (dir.args.size() == 0)
			PERR_EXIT(s_cleanup(NULL), "Error: No allowed methods defined");
		s_set_methods(dir.args, location);
	}
	else if (dir.name == "index") {
		if (dir.args.size() != 1)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid index");
		location.index = dir.args.at(0);
	}
	else if (dir.name == "upload_store") {
		if (dir.args.size() != 1)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid upload store");
		location.upload_store = dir.args.at(0);
	}
	else if (dir.name == "return") {
		if (dir.args.size() != 2)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid redirect");
		HTTP_STATUS(300);
		location.redirect = HTTP::Status::i300;
	}
	else
		PERR_EXIT(s_cleanup(NULL), "Error: Invalid location directive");
	return 0;
}

static inline
isize s_parse_location(tokIter &cursor, tokIter &end, HTTP::Location &loc) {
	s_match(cursor, "location");
	if (cursor->type != Token::WORD)
		PERR_EXIT(s_cleanup(NULL), "Error: Expected location");
	loc.path = cursor->value;
	cursor++;
	s_match(cursor, "{");
	while (cursor != end && cursor->type != Token::CLOSE_BRACKET) {
		Directive dir;
		s_parse_directive(cursor, end, dir);
		s_set_location_directive(dir, loc);
		cursor++;
	}
	if (cursor->type != Token::CLOSE_BRACKET)
		PERR_EXIT(s_cleanup(NULL), "Error: Unexpected token");
	return 0;
}

static inline
isize s_set_server_directive(Directive &dir, HTTP::ServerConfig &server) {
	if (dir.name == "listen") {
		if (server.port != -1 || dir.args.size() != 1)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid port definition");
		server.port = s_strtol(dir.args.at(0));
		if (server.port < 1 || server.port > 65535)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid port");
	}
	else if (dir.name == "host") {
		if (server.host != "localhost" || dir.args.size() != 1)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid host definition");
		server.host = dir.args.at(0);
	}
	else if (dir.name == "client_max_body_size") {
		if (server.maxBodySize != SIZE_MAX || dir.args.size() != 1)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid max body size definition");
		server.maxBodySize = s_strtol(dir.args.at(0));
		if (server.maxBodySize < 1 || server.maxBodySize > 20)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid max body size");
	}
	// TODO: Auto index is only used in locations
	// else if (dir.name == "autoindex") {
	// 	if (dir.args.size() != 1 || (dir.args.at(0) != "on" && dir.args.at(0) != "off"))
	// 		PERR_EXIT(s_cleanup(NULL), "Error: Invalid autoindex");
	// 	server.autoindex = dir.args.at(0) == "on" ? true : false;
	// }	// TODO: this will be different
	else if (dir.name == "error_page") {
		if (dir.args.size() != 2)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid error page");
		i64	error = s_strtol(dir.args.at(0));
		if (error < 400 || error > 599)
			PERR_EXIT(s_cleanup(NULL), "Error: Invalid error number");
		server.errors[error] = dir.args.at(1);
	}
	else
		PERR_EXIT(s_cleanup(NULL), "Error: Invalid server directive");
	return 0;
}

static inline
isize s_parse_server(tokIter cursor, tokIter end, HTTP::ServerConfig &server) {
	s_match(cursor, "server");
	s_match(cursor, "{");
	if (cursor == end || cursor->value == "}")
		PERR_EXIT(s_cleanup(NULL), "Error: Empty server block");
	while (cursor != end) {
		if (cursor->value == "location") {	// TODO: can they be case insenstive?
			HTTP::Location loc;
			s_parse_location(cursor, end, loc);
			server.locations.push_back(loc);
		}
		else {
			Directive dir;
			s_parse_directive(cursor, end, dir);
			s_set_server_directive(dir, server);
		}
		s_advance(cursor, end);
	}
	if (end->type != Token::CLOSE_BRACKET)
		PERR_EXIT(s_cleanup(NULL), "Error: Unexpected token");
	return 0;
}

std::vector<ServerConfig> parse_config(char *fileBuffer, usize totalBytes) {
	s_cleanup(fileBuffer);	// Sets the pointer to be cleaned up

	std::vector<Token> tokVector = tokenizer(fileBuffer);
	tokIter it = tokVector.begin();
	tokIter end = tokVector.end();

	std::vector<ServerConfig> cfgVector;
	cfgVector.reserve(s_count_servers(fileBuffer, totalBytes));

	//tokenizerDump(tokens);
	while (it != end) {
		if (it->value == "server") {
			usize distance = s_find_scope_end(it, end);
			HTTP::ServerConfig serverConf;
			s_parse_server(it, it + distance, serverConf);
			cfgVector.push_back(serverConf);
			it = it + distance;
		}
		else
			PERR_EXIT(s_cleanup(NULL), "Error: Unexpected token");
		it++;
	}
	//configDump(ret);

	return cfgVector;
}

//
}	// Namespace Parse
}	// Namespace HTTP