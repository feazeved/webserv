#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>

#include "parseConfig.hpp"
#include "HTTP.hpp"
#include "Server.hpp"

using namespace parseConfig;

static std::vector<token> tokenizer(std::stringstream &config)
{
    std::vector<token> ret;
	std::string tk;
	token tkn;
	int braces = 0;

	if(config.peek() == EOF){
        throw std::runtime_error("Empty file");
    }

	while (config >> tk)
	{
		size_t pos = tk.find_first_of("{};");
		if(pos != std::string::npos)
		{
		    char delimiter = tk[pos];
		    if(pos + 1 != tk.size()){
			   throw std::runtime_error("Sintax error");
			}
			if(pos != 0)
			{
				tkn.type = WORD;
				tkn.value = tk.substr(0, pos);
				ret.push_back(tkn);
			}
			switch (delimiter) {
				case '{' :
				    tkn.type = OPEN_BRACKET;
					tkn.value = '{';
					braces++;
					break;
				case '}' :
				    tkn.type = CLOSE_BRACKET;
					tkn.value = '}';
					if(--braces < 0)
	                    throw std::runtime_error("Extraneous closing brace ('}')");
					break;
				case ';' :
    				if(tkn.type == SEMICOLON)
                        throw std::runtime_error("Extraneous semicolon (';')");
    				tkn.type = SEMICOLON;
    				tkn.value = ';';
    				break;
				default: throw std::runtime_error("Invalid delimiter"); break;
			}
		}
		else {
			tkn.type = WORD;
			tkn.value = tk;
		}
		ret.push_back(tkn);
	}
	if(braces)
            throw std::runtime_error("Expected '}' to math previous '{'");

	return ret;
}

void match(std::vector<token>::const_iterator &cursor, std::string value){
	if(cursor->value != value)
		throw std::runtime_error("Unexpected token");
	cursor++;
}

void advance(std::vector<token>::const_iterator &cursor, std::vector<token>::const_iterator &end)
{
    if(cursor == end)
        throw std::runtime_error("Invalid read");
    cursor++;
}

void parseDirective(std::vector<token>::const_iterator &cursor, std::vector<token>::const_iterator &end, Directive &dir){
	std::vector<std::string> arguments;

	dir.name = cursor->value;
	cursor++;
	while(cursor != end && cursor->type != parseConfig::SEMICOLON)
	{
		arguments.push_back(cursor->value);
		advance(cursor, end);
		//cursor++;
	}
	if(cursor->type != parseConfig::SEMICOLON)
		throw std::runtime_error("Unexpected token");
	dir.args = arguments;
}

// Alex: Lembra que se nao for referencia aqui ele cria copia
static long stt_strtol(std::string str)
{
	const char *sptr = str.c_str();
	char *eptr = NULL;

	long ret = std::strtol(sptr, &eptr, 10);

	if(*eptr || errno == ERANGE)
		throw std::runtime_error("Invalid directive");

	return (ret);
}

void setMethods(std::vector<std::string> &methods, HTTP::Location &location){
	std::vector<std::string>::iterator it = methods.begin();

	for(; it != methods.end(); it++)
	{
		if(*it != "GET" && *it != "POST" && *it != "DELETE")
			throw std::runtime_error("Invalid method");
		location.methods.push_back(*it);
	}
}

void setLocationDirective(Directive &dir, HTTP::Location &location){
	if(dir.name == "root"){
		if(dir.args.size() != 1)
			throw std::runtime_error("Invalid root");
		location.root = dir.args.at(0);
	}
	else if(dir.name == "autoindex"){
		if(dir.args.size() != 1 || (dir.args.at(0) != "on" && dir.args.at(0) != "off"))
			throw std::runtime_error("Invalid autoindex");
		location.autoindex = dir.args.at(0) == "on" ? true : false;
	}
	else if(dir.name == "allowed_methods"){
		if(dir.args.size() == 0)
			throw std::runtime_error("No allowed methods defined");
		setMethods(dir.args, location);
	}
	else if(dir.name == "index"){
		if(dir.args.size() != 1)
			throw std::runtime_error("Invalid index");
		location.index = dir.args.at(0);
	}
	else if(dir.name == "upload_store"){
		if(dir.args.size() != 1)
			throw std::runtime_error("Invalid index");
		location.upload_store = dir.args.at(0);
	}
	else
		throw std::runtime_error("Invalid location directive");
}

void parseLocation(std::vector<token>::const_iterator &cursor, std::vector<token>::const_iterator &end, HTTP::Location &loc){
	match(cursor, "location");
	if(cursor->type != parseConfig::WORD)
		throw std::runtime_error("Expected location");
	loc.path = cursor->value;
	cursor++;
	match(cursor, "{");
	while(cursor != end && cursor->type != parseConfig::CLOSE_BRACKET)
	{
		Directive dir;
		parseDirective(cursor, end, dir);
		setLocationDirective(dir, loc);
		cursor++;
	}
	if(cursor->type != parseConfig::CLOSE_BRACKET)
		throw std::runtime_error("Unexpected token");
}



void setServerDirective(Directive &dir, HTTP::ServerConfig &server){
	if(dir.name == "listen")
	{
		if(server.port != -1 || dir.args.size() != 1)
			throw std::runtime_error("Invalid port definition");
		server.port = stt_strtol(dir.args.at(0));
		if(server.port < 1 || server.port > 65535)
			throw std::runtime_error("Invalid port");
	}
	else if(dir.name == "client_max_body_size")
	{
		if(server.maxBodySize != -1 || dir.args.size() != 1)
			throw std::runtime_error("Invalid max body size definition");
		server.maxBodySize = stt_strtol(dir.args.at(0));
		if(server.maxBodySize < 1 || server.maxBodySize > 20)
			throw std::runtime_error("Invalid max body size");
	}
	else if (dir.name == "error_page")
	{
		if(dir.args.size() != 2)
			throw std::runtime_error("Invalid error page");
		server.errors[0] = stt_strtol(dir.args.at(0));
		server.errors[1] = dir.args.at(1);
		// if(server.errors[0] < 1 || server.errors.at(0) > 1000)
		//     throw std::runtime_error("Invalid error number");
	}
	else
		throw std::runtime_error("Invalid server directive");
}


void parseServer(std::vector<token>::const_iterator cursor, std::vector<token>::const_iterator end, HTTP::ServerConfig &server){
	match(cursor, "server");
	match(cursor, "{");
	if (cursor == end || cursor->value == "}")
        throw std::runtime_error("Empty server block");
	while (cursor != end) {
		if(cursor->value == "location")
		{
			HTTP::Location loc;
			parseLocation(cursor, end, loc);
			server.locations.push_back(loc);
		}
		else
		{
			Directive dir;
			parseDirective(cursor, end, dir);
			setServerDirective(dir, server);
		}
		advance(cursor, end);
		//cursor++;
	}
	if(end->type != parseConfig::CLOSE_BRACKET)
		throw std::runtime_error("Unexpected token");
}

void tokenizerDump(std::vector<token> &tokens){
	std::vector<token>::iterator it =  tokens.begin();
	std::cout << "---Print tokens---\n\n";
	for(;it != tokens.end(); it++)
	{
		std::string tp;
		switch (it->type) {
			case OPEN_BRACKET: tp = "open bracket"; break;
			case CLOSE_BRACKET: tp = "close bracket"; break;
			case SEMICOLON: tp = "semicolon"; break;
			case WORD: tp = "word"; break;
		}
		std::cout << tp ;
		if(tp == "word")
			std::cout << "(" << it->value << ")";
		std::cout << "\n";
	}
}

void configDump(std::vector<HTTP::ServerConfig> &config){
	std::vector<HTTP::ServerConfig>::iterator it =  config.begin();
	std::cout << "---Print Config---\n\n";
	for(; it != config.end(); it++)
	{
		std::cout << "SERVER\n";
		std::cout << "\tlisten: " << (*it).port << "\n";
		std::cout << "\thost: " << (*it).host << "\n";
		std::cout << "\tmax_body_size: " << (*it).maxBodySize << "\n";
		std::cout << "\n";

		std::vector<HTTP::Location>::iterator itl =  (*it).locations.begin();
		for(; itl != (*it).locations.end(); itl++)
		{
			std::cout << "\tLOCATION " << (*itl).path << "\n";
			std::cout << "\t\troot: " << (*itl).root << "\n";
			std::cout << "\t\tindex: " << (*itl).index << "\n";
			std::cout << "\t\tupload_store: " << (*itl).upload_store << "\n";
			std::cout << "\t\tautoindex: " << ((*itl).autoindex ? "on " : "off ") << "\n";
			std::cout << "\n";
		}
	}

}

size_t scopeEnd(std::vector<token>::iterator &begin, std::vector<token>::iterator &end)
{
    std::vector<token>::iterator it = begin;
    bool startedCount = false;
    int braces = 0;
    size_t distance = 0;

    while(it != end)
    {
        if(it->type == parseConfig::OPEN_BRACKET)
        {
            startedCount = true;
            braces++;
        }
        else if(it->type == parseConfig::CLOSE_BRACKET)
        {
            startedCount = true;
            braces--;
        }
        if(!braces && startedCount)
            break;
        it++;
        distance++;
    }
    return distance;
}

std::vector<HTTP::ServerConfig> parseConfig::parseConfig(char *filePath){
	std::vector<HTTP::ServerConfig> ret;
	std::stringstream   stream;
	std::ifstream inputFile(filePath);

	if(inputFile.is_open())
		stream << inputFile.rdbuf();
	inputFile.close();

	std::vector<token> tokens = tokenizer(stream);
	std::vector<token>::iterator it =  tokens.begin();
	std::vector<token>::iterator end =  tokens.end();

	//tokenizerDump(tokens);

	while(it != end)
	{
		if(it->value == "server")
		{
		    size_t distance = scopeEnd(it, end);
		    HTTP::ServerConfig    serverConf;
			parseServer(it, it + distance, serverConf);
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
