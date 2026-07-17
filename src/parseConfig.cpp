#include <cerrno>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>

#include "parseConfig.hpp"
#include "Http.hpp"
#include "Server.hpp"

using namespace parseConfig;

static std::vector<token> tokenizer(std::stringstream &config)
{
    std::vector<token> ret;
    std::string tk;
    token tkn;

    while (config >> tk)
    {
        size_t pos = tk.find_first_of("{};");
        if(pos != std::string::npos)
        {
            char delimiter = tk[pos];
            if(pos != 0)
            {
                tkn.type = WORD;
                tkn.value = tk.substr(0, pos);
                ret.push_back(tkn);
            }
            switch (delimiter) {
                case '{' : tkn.type = OPEN_BRACKET; tkn.value = '{';	break;
                case '}' : tkn.type = CLOSE_BRACKET; tkn.value = '}';	break;
                case ';' : tkn.type = SEMICOLON; tkn.value = ';';		break;
                default: throw std::runtime_error("Invalid delimiter"); break;
            }
        }
        else {
            tkn.type = WORD;
            tkn.value = tk;
        }
        ret.push_back(tkn);
    }

    return ret;
}

void match(std::vector<token>::const_iterator &cursor, std::string value){
    if(cursor->value != value)
        throw std::runtime_error("Expected " + value);
    cursor++;
}

void parseDirective(std::vector<token>::const_iterator &cursor, std::vector<token>::const_iterator &end, Directive &dir){
    std::vector<std::string> arguments;

    dir.name = cursor->value;
    cursor++;
    while(cursor != end && cursor->type != parseConfig::SEMICOLON)
    {
        arguments.push_back(cursor->value);
        cursor++;
    }
    if(cursor->type != parseConfig::SEMICOLON)
        throw std::runtime_error("Expected semicolon");
    dir.args = arguments;
}

static long stt_strtol(std::string str)
{
    const char *sptr = str.c_str();
    char *eptr = NULL;

    long ret = std::strtol(sptr, &eptr, 10);

    if(*eptr || errno == ERANGE)
        throw std::runtime_error("Invalid directive");

    return (ret);
}

void setMethods(std::vector<std::string> &methods, Http::Location &location){
    std::vector<std::string>::iterator it = methods.begin();

    for(; it != methods.end(); it++)
    {
        if(*it != "GET" && *it != "POST" && *it != "DELETE")
            throw std::runtime_error("Invalid method");
        location.methods.push_back(*it);
    }
}

void setLocationDirective(Directive &dir, Http::Location &location){
    if(dir.name == "root")
    {
        if(dir.args.size() != 1)
            throw std::runtime_error("Invalid root");
        location.root = dir.args.at(0);
    }
    else if(dir.name == "autoindex")
    {
        if(dir.args.size() != 1 || (dir.args.at(0) != "on" && dir.args.at(0) != "off"))
            throw std::runtime_error("Invalid autoindex");
        location.autoindex = dir.args.at(0) == "on" ? true : false;
    }
    else if(dir.name == "allowed_methods")
    {
        if(dir.args.size() == 0)
            throw std::runtime_error("No allowed methods defined");
        setMethods(dir.args, location);
    }
}

void parseLocation(std::vector<token>::const_iterator &cursor, std::vector<token>::const_iterator &end, Http::Location &loc){
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
        cursor++;
    }
    if(cursor->type != parseConfig::CLOSE_BRACKET)
        throw std::runtime_error("Expected close bracket");
}



void setServerDirective(Directive &dir, Http::ServerConfig &server){
    if(dir.name == "listen")
    {
        if(server.port != -1 || dir.args.size() != 1)
            throw std::runtime_error("Duplicate port definition");
        server.port = stt_strtol(dir.args.at(0));
        if(server.port < 1 || server.port > 65535)
            throw std::runtime_error("Invalid port");
    }
    else if(dir.name == "client_max_body_size")
    {
        if(server.maxBodySize != -1 || dir.args.size() != 1)
            throw std::runtime_error("Duplicate max body size definition");
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
}

void parseServer(std::vector<token>::const_iterator cursor, std::vector<token>::const_iterator end, Http::ServerConfig &server){
    match(cursor, "server");
    match(cursor, "{");
    while (cursor != end) {
        if(cursor->value == "location")
        {
            Http::Location loc;
            parseLocation(cursor, end, loc);
            server.locations.push_back(loc);
        }
        else
        {
            Directive dir;
            parseDirective(cursor, end, dir);
            setServerDirective(dir, server);
        }
        cursor++;
    }
    if(end->type != parseConfig::CLOSE_BRACKET)
        throw std::runtime_error("Expected close bracket");
}

std::vector<Http::ServerConfig> parseConfig::parseConfig(char *filePath){
    std::vector<Http::ServerConfig> ret;
    std::stringstream   stream;
    std::ifstream inputFile(filePath);

    if(inputFile.is_open())
        stream << inputFile.rdbuf();
    inputFile.close();

    std::vector<token> tokens = tokenizer(stream);

    std::vector<token>::iterator it =  tokens.begin();

    //  std::cout << "---Print tokens---\n\n";
    // for(;it != tokens.end(); it++)
    // {
    //     std::string tp;
    //     switch (it->type) {
    //         case OPEN_BRACKET: tp = "open bracket"; break;
    //         case CLOSE_BRACKET: tp = "close bracket"; break;
    //         case SEMICOLON: tp = "semicolon"; break;
    //         case WORD: tp = "word"; break;
    //     }
    //     std::cout << tp ;
    //     if(tp == "word")
    //         std::cout << "(" << it->value << ")";
    //     std::cout << "\n";
    // }
    // it =  tokens.begin();

    while(it != tokens.end())
    {
        //std::cout << it->value << "\n";
        if(it->value == "server")
        {
            Http::ServerConfig    serverConf;
            parseServer(it, tokens.end() - 1, serverConf);
            ret.push_back(serverConf);
        }
        it++;
    }

    return ret;
}
