#pragma once
#include "core.hpp"

namespace HTTP {

#define STATUS_100 "100 Continue"
#define STATUS_101 "101 Switching Protocols"
#define STATUS_102 "102 Processing"
#define STATUS_103 "103 Early Hints"
#define STATUS_104 "104 Upload Resumption Supported"
#define STATUS_200 "200 OK"
#define STATUS_201 "201 Created"
#define STATUS_202 "202 Accepted"
#define STATUS_203 "203 Non-Authoritative Information"
#define STATUS_204 "204 No Content"
#define STATUS_205 "205 Reset Content"
#define STATUS_206 "206 Partial Content"
#define STATUS_207 "207 Multi-Status"
#define STATUS_208 "208 Already Reported"
#define STATUS_226 "226 IM Used"
#define STATUS_300 "300 Multiple Choices"
#define STATUS_301 "301 Moved Permanently"
#define STATUS_302 "302 Found"
#define STATUS_303 "303 See Other"
#define STATUS_304 "304 Not Modified"
#define STATUS_305 "305 Use Proxy"
#define STATUS_306 "306 (Unused)"
#define STATUS_307 "307 Temporary Redirect"
#define STATUS_308 "308 Permanent Redirect"
#define STATUS_400 "400 Bad Request"
#define STATUS_401 "401 Unauthorized"
#define STATUS_402 "402 Payment Required"
#define STATUS_403 "403 Forbidden"
#define STATUS_404 "404 Not Found"
#define STATUS_405 "405 Method Not Allowed"
#define STATUS_406 "406 Not Acceptable"
#define STATUS_407 "407 Proxy Authentication Required"
#define STATUS_408 "408 Request Timeout"
#define STATUS_409 "409 Conflict"
#define STATUS_410 "410 Gone"
#define STATUS_411 "411 Length Required"
#define STATUS_412 "412 Precondition Failed"
#define STATUS_413 "413 Content Too Large"
#define STATUS_414 "414 URI Too Long"
#define STATUS_415 "415 Unsupported Media Type"
#define STATUS_416 "416 Range Not Satisfiable"
#define STATUS_417 "417 Expectation Failed"
#define STATUS_418 "418 (Unused)"
#define STATUS_421 "421 Misdirected Request"
#define STATUS_422 "422 Unprocessable Content"
#define STATUS_423 "423 Locked"
#define STATUS_424 "424 Failed Dependency"
#define STATUS_425 "425 Too Early"
#define STATUS_426 "426 Upgrade Required"
#define STATUS_428 "428 Precondition Required"
#define STATUS_429 "429 Too Many Requests"
#define STATUS_431 "431 Request Header Fields Too Large"
#define STATUS_451 "451 Unavailable For Legal Reasons" // not used, would make the lut too big
#define STATUS_500 "500 Internal Server Error"
#define STATUS_501 "501 Not Implemented"
#define STATUS_502 "502 Bad Gateway"
#define STATUS_503 "503 Service Unavailable"
#define STATUS_504 "504 Gateway Timeout"
#define STATUS_505 "505 HTTP Version Not Supported"
#define STATUS_506 "506 Variant Also Negotiates"
#define STATUS_507 "507 Insufficient Storage"
#define STATUS_508 "508 Loop Detected"
#define STATUS_510 "510 Not Extended"
#define STATUS_511 "511 Network Authentication Required"

#define STATUS(code) STATUS_##code
enum
{
	i000 = 1,
	i100 = 3,
	i101 = i100 + sizeof(STATUS(100)) + 1,
	i102 = i101 + sizeof(STATUS(101)) + 1,
	i103 = i102 + sizeof(STATUS(102)) + 1,
	i104 = i103 + sizeof(STATUS(103)) + 1,
	i200 = i104 + sizeof(STATUS(104)) + 1,
	i201 = i200 + sizeof(STATUS(200)) + 1,
	i202 = i201 + sizeof(STATUS(201)) + 1,
	i203 = i202 + sizeof(STATUS(202)) + 1,
	i204 = i203 + sizeof(STATUS(203)) + 1,
	i205 = i204 + sizeof(STATUS(204)) + 1,
	i206 = i205 + sizeof(STATUS(205)) + 1,
	i207 = i206 + sizeof(STATUS(206)) + 1,
	i208 = i207 + sizeof(STATUS(207)) + 1,
	i226 = i208 + sizeof(STATUS(208)) + 1,
	i300 = i226 + sizeof(STATUS(226)) + 1,
	i301 = i300 + sizeof(STATUS(300)) + 1,
	i302 = i301 + sizeof(STATUS(301)) + 1,
	i303 = i302 + sizeof(STATUS(302)) + 1,
	i304 = i303 + sizeof(STATUS(303)) + 1,
	i305 = i304 + sizeof(STATUS(304)) + 1,
	i306 = i305 + sizeof(STATUS(305)) + 1,
	i307 = i306 + sizeof(STATUS(306)) + 1,
	i308 = i307 + sizeof(STATUS(307)) + 1,
	i400 = i308 + sizeof(STATUS(308)) + 1,
	i401 = i400 + sizeof(STATUS(400)) + 1,
	i402 = i401 + sizeof(STATUS(401)) + 1,
	i403 = i402 + sizeof(STATUS(402)) + 1,
	i404 = i403 + sizeof(STATUS(403)) + 1,
	i405 = i404 + sizeof(STATUS(404)) + 1,
	i406 = i405 + sizeof(STATUS(405)) + 1,
	i407 = i406 + sizeof(STATUS(406)) + 1,
	i408 = i407 + sizeof(STATUS(407)) + 1,
	i409 = i408 + sizeof(STATUS(408)) + 1,
	i410 = i409 + sizeof(STATUS(409)) + 1,
	i411 = i410 + sizeof(STATUS(410)) + 1,
	i412 = i411 + sizeof(STATUS(411)) + 1,
	i413 = i412 + sizeof(STATUS(412)) + 1,
	i414 = i413 + sizeof(STATUS(413)) + 1,
	i415 = i414 + sizeof(STATUS(414)) + 1,
	i416 = i415 + sizeof(STATUS(415)) + 1,
	i417 = i416 + sizeof(STATUS(416)) + 1,
	i418 = i417 + sizeof(STATUS(417)) + 1,
	i421 = i418 + sizeof(STATUS(418)) + 1,
	i422 = i421 + sizeof(STATUS(421)) + 1,
	i423 = i422 + sizeof(STATUS(422)) + 1,
	i424 = i423 + sizeof(STATUS(423)) + 1,
	i425 = i424 + sizeof(STATUS(424)) + 1,
	i426 = i425 + sizeof(STATUS(425)) + 1,
	i428 = i426 + sizeof(STATUS(426)) + 1,
	i429 = i428 + sizeof(STATUS(428)) + 1,
	i431 = i429 + sizeof(STATUS(429)) + 1,
	i500 = i431 + sizeof(STATUS(431)) + 1,
	i501 = i500 + sizeof(STATUS(500)) + 1,
	i502 = i501 + sizeof(STATUS(501)) + 1,
	i503 = i502 + sizeof(STATUS(502)) + 1,
	i504 = i503 + sizeof(STATUS(503)) + 1,
	i505 = i504 + sizeof(STATUS(504)) + 1,
	i506 = i505 + sizeof(STATUS(505)) + 1,
	i507 = i506 + sizeof(STATUS(506)) + 1,
	i508 = i507 + sizeof(STATUS(507)) + 1,
	i510 = i508 + sizeof(STATUS(508)) + 1,
	i511 = i510 + sizeof(STATUS(510)) + 1
};

static const u16 s_offsets[160] =
{
	i100, i101, i102, i103, i104, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000,
	i200, i201, i202, i203, i204, i205, i206, i207,
	i208, i000, i000, i000, i000, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000,
	i000, i000, i226, i000, i000, i000, i000, i000,
	i300, i301, i302, i303, i304, i305, i306, i307,
	i308, i000, i000, i000, i000, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000,
	i400, i401, i402, i403, i404, i405, i406, i407,
	i408, i409, i410, i411, i412, i413, i414, i415,
	i416, i417, i418, i000, i000, i421, i422, i423,
	i424, i425, i426, i000, i428, i429, i000, i431,
	i500, i501, i502, i503, i504, i505, i506, i507,
	i508, i000, i510, i511, i000, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000,
	i000, i000, i000, i000, i000, i000, i000, i000
};

static const char s_statusCodes[] = "\0\0"
	"\x0C" STATUS(100) "\0"	"\x17" STATUS(101) "\0"	"\x0E" STATUS(102) "\0"
	"\x0F" STATUS(103) "\0"	"\x1F" STATUS(104) "\0"	"\x06" STATUS(200) "\0"

	"\x0B" STATUS(201) "\0"	"\x0C" STATUS(202) "\0"	"\x21" STATUS(203) "\0"
	"\x0E" STATUS(204) "\0"	"\x11" STATUS(205) "\0"	"\x13" STATUS(206) "\0"
	"\x10" STATUS(207) "\0"	"\x14" STATUS(208) "\0"	"\x0B" STATUS(226) "\0"

	"\x14" STATUS(300) "\0"	"\x15" STATUS(301) "\0"	"\x09" STATUS(302) "\0"
	"\x0D" STATUS(303) "\0"	"\x10" STATUS(304) "\0"	"\x0D" STATUS(305) "\0"
	"\x0C" STATUS(306) "\0"	"\x16" STATUS(307) "\0"	"\x16" STATUS(308) "\0"

	"\x0F" STATUS(400) "\0"	"\x10" STATUS(401) "\0"	"\x14" STATUS(402) "\0"
	"\x0D" STATUS(403) "\0"	"\x0D" STATUS(404) "\0"	"\x16" STATUS(405) "\0"
	"\x12" STATUS(406) "\0"	"\x21" STATUS(407) "\0"	"\x13" STATUS(408) "\0"
	"\x0C" STATUS(409) "\0"	"\x08" STATUS(410) "\0"	"\x13" STATUS(411) "\0"
	"\x17" STATUS(412) "\0"	"\x15" STATUS(413) "\0"	"\x10" STATUS(414) "\0"
	"\x1A" STATUS(415) "\0"	"\x19" STATUS(416) "\0"	"\x16" STATUS(417) "\0"
	"\x0C" STATUS(418) "\0"	"\x17" STATUS(421) "\0"	"\x19" STATUS(422) "\0"
	"\x0A" STATUS(423) "\0"	"\x15" STATUS(424) "\0"	"\x0D" STATUS(425) "\0"
	"\x14" STATUS(426) "\0"	"\x19" STATUS(428) "\0"	"\x15" STATUS(429) "\0"
	"\x23" STATUS(431) "\0"

	"\x19" STATUS(500) "\0"	"\x13" STATUS(501) "\0"	"\x0F" STATUS(502) "\0"
	"\x17" STATUS(503) "\0"	"\x13" STATUS(504) "\0"	"\x1E" STATUS(505) "\0"
	"\x1B" STATUS(506) "\0"	"\x18" STATUS(507) "\0"	"\x11" STATUS(508) "\0"
	"\x10" STATUS(510) "\0"	"\x23" STATUS(511) "\0";

class Status {
public:
	u16 index;

	Status(usize number) {
		usize div = number / 100;
		usize rem = number - div * 100;

		index = (number < 100 || number >= 512 || rem >= 32) 
			? 1 : s_offsets[(div - 1) * 32 + rem];
	}

	const char *c_str() {
		return s_statusCodes + index;
	}

	usize size() {
		return (usize)s_statusCodes[index - 1];
	}
};
}

