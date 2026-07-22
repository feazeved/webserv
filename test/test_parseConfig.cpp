#include "catch.hpp"
#include <vector>
#include <iostream>
#include "parseConfig.hpp"
#include "HTTP.hpp"


TEST_CASE("config file parsing basics", "[parser]") {

	std::vector<HTTP::ServerConfig> servers = parseConfig::parseConfig("config/default.conf");


	int	port = servers.at(0).port;

	SECTION("Port validation logic") {
		CHECK(port > 0);
		CHECK(port <= 65535);
	}
}

TEST_CASE("all invalid files", "[parser]") {
    std::vector<std::string> paths;

   SECTION("default"){
        CHECK_NOTHROW(parseConfig::parseConfig("config/default.config"));
   }
    SECTION("empty"){
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/emptyFile.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/emptyDirectiveArgument.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/emptyLocationPath.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/emptyServerBlock.config"));
    }
    SECTION("missing"){
        CHECK_THROWS(parseConfig::parseConfig("config/invalid/missingClosingBrace.config"));
        CHECK_THROWS(parseConfig::parseConfig("config/invalid/missingOpeningBrace.config"));
        CHECK_THROWS(parseConfig::parseConfig("config/invalid/missingSemicolon.config"));
    }
    SECTION("mutiple"){
        CHECK_THROWS(parseConfig::parseConfig("config/invalid/multipleListen.config"));
        CHECK_THROWS(parseConfig::parseConfig("config/invalid/multipleServers.config"));
    }
    SECTION("extra funciona 2"){
        CHECK_THROWS(parseConfig::parseConfig("config/invalid/deeplyNestedInvalidBlock.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/extraSemicolon.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/invalidPort.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/nestedLocastion.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/portTooLarge.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/portZero.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/unexpectedToken.config"));
       CHECK_THROWS(parseConfig::parseConfig("config/invalid/unknownDirective.config"));
    }
}
