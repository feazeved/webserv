#include <algorithm>
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


void parseServer(std::vector<token>::const_iterator cursor, std::vector<token>::const_iterator end, ServerConfig &server){
    if(cursor->value != "server")
        throw std::runtime_error("Expected server");
    cursor++;
    if(cursor->type != parseConfig::OPEN_BRACKET)
        throw std::runtime_error("Expected open bracket");
    else
    {
        cursor++;
       	// if(cursor->value == "location")
        //      parseLocation();
        // else
        //      parseDirective();
        Directive dir;
        std::vector<std::string> arguments;
        if(cursor->value == "listen")
        {
            dir.name = cursor->value;
            cursor++;
            while(cursor->type != parseConfig::SEMICOLON)
            {
                arguments.push_back(cursor->value);
                cursor++;
            }
            dir.args = arguments;

            server.setPort(std::atoi(dir.args.at(0).c_str()));
        }
    }
    if(end->type != parseConfig::CLOSE_BRACKET)
        throw std::runtime_error("Expected close bracket");
}

// void parseLocation(std::vector<token>::const_iterator cursor, std::vector<token>::const_iterator end){
//     Location loc;
//     if(cursor->value != "location")
//         throw std::runtime_error("Expected location");
//     cursor++;
//     if(cursor->type != parseConfig::WORD)
//         throw std::runtime_error("Expected location");
//     // else
//     //     alias
// }

std::vector<ServerConfig> parseConfig::parseConfig(char *filePath){
    std::vector<ServerConfig> ret;
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
            ServerConfig    serverConf;
            parseServer(it, tokens.end() - 1, serverConf);
            ret.push_back(serverConf);
        }
        it++;
    }

    return ret;
}
