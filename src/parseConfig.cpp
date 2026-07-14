#include <iostream>
#include <fstream>

#include "parseConfig.hpp"

using namespace parseConfig;

std::vector<token> parseConfig::tokenizer(std::stringstream &config)
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
                case '{' : tkn.type = OPEN_BRACKET; break;
                case '}' : tkn.type = CLOSE_BRACKET; break;
                case ';' : tkn.type = SEMICOLON; break;
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

std::vector<ServerConfig> parseConfig::parseConfig(char *filePath){
    std::vector<ServerConfig> ret;
    std::stringstream   stream;
    std::ifstream inputFile(filePath);

    if(inputFile.is_open())
        stream << inputFile.rdbuf();
    inputFile.close();

    std::vector<token> tokens = tokenizer(stream);

    std::vector<token>::iterator it =  tokens.begin();
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

    return ret;
}
