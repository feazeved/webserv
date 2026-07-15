#pragma once

#include <string>
#include <vector>

#include "Location.hpp"

class ServerConfig {
    private:
        std::vector<Location>		locations;
        std::vector<std::string>	errors;
        std::string					host;
        int							port;
        int							maxBodySize;

    public:
        int		getPort() const { return (port); }
        void	setPort(const int p) { port = p; }

        int		getMaxBodySize() const { return (maxBodySize); }
        void	setMaxBodySize(const int mbs) { maxBodySize = mbs; }
};
