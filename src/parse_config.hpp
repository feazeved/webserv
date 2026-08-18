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
//
static
std::vector<ParseToken> tokenizer(std::stringstream &config) {
	std::vector<ParseToken> ret;
	std::string tk;
	ParseToken tkn;
	int braces = 0;

	if (config.peek() == EOF)
		throw std::runtime_error("Empty file");

	while (config >> tk) {
		usize pos = tk.find_first_of("{};");
		if (pos != std::string::npos) {
			char delimiter = tk[pos];
			if (pos + 1 != tk.size())
				throw std::runtime_error("Syntax error");
			if (pos != 0)
			{
				tkn.type = Token::WORD;
				tkn.value = tk.substr(0, pos);
				ret.push_back(tkn);
			}
			switch (delimiter) {
				case '{' :
					tkn.type = Token::OPEN_BRACKET;
					tkn.value = '{';
					braces++;
					break;
				case '}' :
					tkn.type = Token::CLOSE_BRACKET;
					tkn.value = '}';
					if (--braces < 0)
						throw std::runtime_error("Extraneous closing brace ('}')");
					break;
				case ';' :
								if (tkn.type == Token::SEMICOLON)
						throw std::runtime_error("Extraneous semicolon (';')");
								tkn.type = Token::SEMICOLON;
								tkn.value = ';';
								break;
				default: throw std::runtime_error("Invalid delimiter"); break;
			}
		}
		else {
			tkn.type = Token::WORD;
			tkn.value = tk;
		}
		ret.push_back(tkn);
	}
	if (braces)
		throw std::runtime_error("Expected '}' to match previous '{'");

	return ret;
}

void parse_directive(tokIter &cursor, tokIter &end, ParseDirective &dir) {
	std::vector<std::string> arguments;

	dir.name = cursor->value;
	cursor++;
	while (cursor != end && cursor->type != Token::SEMICOLON)
	{
		arguments.push_back(cursor->value);
		s_advance(cursor, end);
	}
	if (cursor->type != Token::SEMICOLON)
		throw std::runtime_error("Unexpected token");
	dir.args = arguments;
}

void set_methods(std::vector<std::string> &methods, HTTP::Location &location) {
	std::vector<std::string>::iterator it = methods.begin();

	for (; it != methods.end(); it++)
	{
		if (it->size() == 3 && MEMCMP(it->c_str(), "GET", 3) == 0)
			location.methods |= HTTP::Mode::GET;
		else if (it->size() == 4 && MEMCMP(it->c_str(), "POST", 4) == 0)
			location.methods |= HTTP::Mode::POST;
		else if (it->size() == 6 && MEMCMP(it->c_str(), "DELETE", 6) == 0)
			location.methods |= HTTP::Mode::DELETE;
		else
			throw std::runtime_error("Invalid method");
	}
}

void set_location_directive(ParseDirective &dir, HTTP::Location &location) {
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
		set_methods(dir.args, location);
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
}

void parse_location(tokIter &cursor, tokIter &end, HTTP::Location &loc) {
	s_match(cursor, "location");
	if (cursor->type != Token::WORD)
		throw std::runtime_error("Expected location");
	loc.path = cursor->value;
	cursor++;
	s_match(cursor, "{");
	while (cursor != end && cursor->type != Token::CLOSE_BRACKET)
	{
		ParseDirective dir;
		parse_directive(cursor, end, dir);
		set_location_directive(dir, loc);
		cursor++;
	}
	if (cursor->type != Token::CLOSE_BRACKET)
		throw std::runtime_error("Unexpected token");
}

void set_server_directive(ParseDirective &dir, HTTP::ServerConfig &server) {
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
}

void parse_server(tokIter cursor, tokIter end, HTTP::ServerConfig &server) {
	s_match(cursor, "server");
	s_match(cursor, "{");
	if (cursor == end || cursor->value == "}")
		throw std::runtime_error("Empty server block");
	while (cursor != end) {
		if (cursor->value == "location")
		{
			HTTP::Location loc;
			parse_location(cursor, end, loc);
			server.locations.push_back(loc);
		}
		else {
			ParseDirective dir;
			parse_directive(cursor, end, dir);
			set_server_directive(dir, server);
		}
		s_advance(cursor, end);
	}
	if (end->type != Token::CLOSE_BRACKET)
		throw std::runtime_error("Unexpected token");
}

usize scope_end(tokIter &begin, tokIter &end) {
	tokIter it = begin;
	bool startedCount = false;
	int braces = 0;
	usize distance = 0;

	while (it != end)
	{
		if (it->type == Token::OPEN_BRACKET)
		{
			startedCount = true;
			braces++;
		}
		else if (it->type == Token::CLOSE_BRACKET)
		{
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

std::vector<HTTP::ServerConfig> parse_config(char *filePath) {
	std::vector<HTTP::ServerConfig> ret;
	std::stringstream stream;
	std::ifstream inputFile(filePath);

	if (inputFile.is_open())
		stream << inputFile.rdbuf();
	inputFile.close();

	std::vector<ParseToken> tokens = tokenizer(stream);
	tokIter it = tokens.begin();
	tokIter end = tokens.end();

	//tokenizerDump(tokens);

	while (it != end) {
		if (it->value == "server")
		{
			usize distance = scope_end(it, end);
			HTTP::ServerConfig    serverConf;
			parse_server(it, it + distance, serverConf);
			ret.push_back(serverConf);
			it = it + distance;
		}
		else
			throw std::runtime_error("Unexpected token");
		it++;
	}

	//configDump(ret);

	return ret;
}
}
