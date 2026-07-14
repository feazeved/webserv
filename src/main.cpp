#include "../includes/ServerConfig.hpp"
#include <vector>

#include "parseConfig.hpp"

int main(int argc, char **argv)
{
    if(argc != 2 || !argv[1][0])
        return 1;

    std::vector<ServerConfig> config = parseConfig::parseConfig(argv[1]);
}
