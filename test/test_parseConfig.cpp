#include "catch.hpp"

#include <vector>
#include "parseConfig.hpp"
#include "ServerConfig.hpp"


TEST_CASE("config file parsing basics", "[parser]") {

	std::vector<ServerConfig> servers = parseConfig::parseConfig("config/default.conf");

	ServerConfig	test = servers[0];

	int	port = test.getPort();

	SECTION("Port validation logic") {
		REQUIRE(port > 0);
		REQUIRE(port <= 65535);
	}
}
