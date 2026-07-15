#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstdlib>

#include "parseConfig.hpp"

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
        //loop setter fuction pointer
        cursor++;
    }
    if(cursor->type != parseConfig::CLOSE_BRACKET)
        throw std::runtime_error("Expected close bracket");
}

void parseServer(std::vector<token>::const_iterator cursor, std::vector<token>::const_iterator end, Http::ServerConfig &server){
    match(cursor, "server");
    //function match to repetitive check, avoinding nests. Ex: expected(cursor, '{') { match ? cursor++ : throw}
    match(cursor, "{");
    while (cursor != end) {
        if(cursor->value == "location")
        {
            Http::Location loc;
            parseLocation(cursor, end, loc);
        }
        //else
        // parseDirective();
        else if (cursor->value == "listen")
        {
            Directive dir;
            parseDirective(cursor, end, dir);
            // maybe a loop with fuction pointer to server's directive setters
            server.port = std::atoi(dir.args.at(0).c_str());
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
