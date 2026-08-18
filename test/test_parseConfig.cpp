#include "catch.hpp"
#include <vector>
#include <iostream>
#include "parse_config.hpp"
#include "HTTP.hpp"


TEST_CASE("config file parsing basics", "[parser]") {

	std::vector<HTTP::ServerConfig> servers = parse_config::parse_config("config/default.conf");


	int	port = servers.at(0).port;

	SECTION("Port validation logic") {
		CHECK(port > 0);
		CHECK(port <= 65535);
	}
}

TEST_CASE("all invalid files", "[parser]") {
    std::vector<std::string> paths;

    SECTION("empty"){
        CHECK_THROWS(parse_config::parse_config("config/invalid/emptyFile.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/emptyDirectiveArgument.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/emptyLocationPath.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/emptyServerBlock.config"));
    }
    SECTION("missing"){
        CHECK_THROWS(parse_config::parse_config("config/invalid/missingClosingBrace.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/missingClosingBrace2.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/missingOpeningBrace.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/missingOpeningBrace2.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/missingSemicolon.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/missingSemicolon2.config"));
    }
    SECTION("mutiple"){
        CHECK_THROWS(parse_config::parse_config("config/invalid/multipleListen.config"));
    }
    SECTION("extra"){
        CHECK_THROWS(parse_config::parse_config("config/invalid/extraSemicolon.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/extraSemicolon2.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/extraClosingBrace.config"));
    }
    SECTION("extra"){
        CHECK_THROWS(parse_config::parse_config("config/invalid/deeplyNestedInvalidBlock.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/invalidPort.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/nestedLocastion.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/portTooLarge.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/portZero.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/unexpectedToken.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/unexpectedToken2.config"));
        CHECK_THROWS(parse_config::parse_config("config/invalid/unknownDirective.config"));
    }
}
