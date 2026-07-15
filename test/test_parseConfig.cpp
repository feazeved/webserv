#include "catch.hpp"
#include <vector>
#include "parseConfig.hpp"
#include "Http.hpp"


TEST_CASE("config file parsing basics", "[parser]") {

	std::vector<Http::ServerConfig> servers = parseConfig::parseConfig("config/default.conf");


	int	port = 8080;

	SECTION("Port validation logic") {
		CHECK(port > 0);
		CHECK(port <= 65535);
	}
}
