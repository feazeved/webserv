#include "catch.hpp"
#include <vector>
#include "parseConfig.hpp"
#include "ServerConfig.hpp"


TEST_CASE("config file parsing basics", "[parser]") {

	std::vector<ServerConfig> servers = parseConfig::parseConfig("config/default.conf");

	ServerConfig	test = servers[0];

	int	port = test.getPort();

	SECTION("Port validation logic") {
		CHECK(port > 0);
		CHECK(port <= 65535);
	}
}
