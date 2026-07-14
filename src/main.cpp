#include "../includes/ServerConfig.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>

// std::vector<Location>		locations;
// std::vector<std::string>	errors;
// std::string					host;
// int							port;
// int							maxBodySize;

enum tokenType{
    OPEN_BRACKET,
    CLOSE_BRACKET,
    SEMICOLON,
    WORD
};

struct token{
    tokenType type;
    std::string value;
};

std::vector<token> tokenizer(std::stringstream &config)
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

std::vector<ServerConfig> parseConfig(char *filePath){
    std::vector<ServerConfig> ret;
    std::stringstream   stream;
    std::ifstream inputFile(filePath);

    if(inputFile.is_open())
        stream << inputFile.rdbuf();
    inputFile.close();

    std::vector<token> tokens = tokenizer(stream);

    std::vector<token>::iterator it =  tokens.begin();
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

    return ret;
}

int main(int argc, char **argv)
{
    if(argc != 2 || !argv[1][0])
        return 1;

    std::vector<ServerConfig> config = parseConfig(argv[1]);
}
