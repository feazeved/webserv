#include <exception>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "parseConfig.hpp"
#include "Location.hpp"
#include "ServerConfig.hpp"

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
                case '{' : tkn.type = OPEN_BRACKET; tkn.value = tk;		break;
                case '}' : tkn.type = CLOSE_BRACKET; tkn.value = tk;	break;
                case ';' : tkn.type = SEMICOLON; tkn.value = tk;		break;
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


void parseServer(std::vector<token>::const_iterator cursor, std::vector<token>::const_iterator end){
	if(cursor->value != "server")
		throw std::runtime_error("Expected server");
	cursor++;
	if(cursor->type != parseConfig::OPEN_BRACKET)
		throw std::runtime_error("Expected open bracket");
	if(end->type != parseConfig::CLOSE_BRACKET)
		throw std::runtime_error("Expected close bracket");
}

void parseLocation(std::vector<token>::const_iterator cursor, std::vector<token>::const_iterator end){
    Location loc;
    if(cursor->value != "location")
        throw std::runtime_error("Expected location");
    cursor++;
    if(cursor->type != parseConfig::WORD)
        throw std::runtime_error("Expected location");
    // else
    //     alias

}

std::vector<ServerConfig> parseConfig::parseConfig(char *filePath){
    ServerConfig    server;
    std::vector<ServerConfig> ret;
    std::stringstream   stream;
    std::ifstream inputFile(filePath);

    if(inputFile.is_open())
        stream << inputFile.rdbuf();
    inputFile.close();

    std::vector<token> tokens = tokenizer(stream);


    //(void)it;
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

    std::vector<token>::const_iterator it =  tokens.begin();
    for(;it != tokens.end(); it++)
    {
    	std::cout << it->type << "\n";
    	if(it->value == "server")
     		parseServer(it, tokens.end() - 1);
        // else if(it->value == "location")
        //     parseLocation(it, tokens.end() - 1);
    }

    return ret;
}
