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
// #define STATUS_451 "451 Unavailable For Legal Reasons" would make the lut too big

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
	idx100 = 3,
	idx101 = idx100 + sizeof(STATUS(100)) + 1,
	idx102 = idx101 + sizeof(STATUS(101)) + 1,
	idx103 = idx102 + sizeof(STATUS(102)) + 1,
	idx104 = idx103 + sizeof(STATUS(103)) + 1,
	idx200 = idx104 + sizeof(STATUS(104)) + 1,
	idx201 = idx200 + sizeof(STATUS(200)) + 1,
	idx202 = idx201 + sizeof(STATUS(201)) + 1,
	idx203 = idx202 + sizeof(STATUS(202)) + 1,
	idx204 = idx203 + sizeof(STATUS(203)) + 1,
	idx205 = idx204 + sizeof(STATUS(204)) + 1,
	idx206 = idx205 + sizeof(STATUS(205)) + 1,
	idx207 = idx206 + sizeof(STATUS(206)) + 1,
	idx208 = idx207 + sizeof(STATUS(207)) + 1,
	idx226 = idx208 + sizeof(STATUS(208)) + 1,
	idx300 = idx226 + sizeof(STATUS(226)) + 1,
	idx301 = idx300 + sizeof(STATUS(300)) + 1,
	idx302 = idx301 + sizeof(STATUS(301)) + 1,
	idx303 = idx302 + sizeof(STATUS(302)) + 1,
	idx304 = idx303 + sizeof(STATUS(303)) + 1,
	idx305 = idx304 + sizeof(STATUS(304)) + 1,
	idx306 = idx305 + sizeof(STATUS(305)) + 1,
	idx307 = idx306 + sizeof(STATUS(306)) + 1,
	idx308 = idx307 + sizeof(STATUS(307)) + 1,
	idx400 = idx308 + sizeof(STATUS(308)) + 1,
	idx401 = idx400 + sizeof(STATUS(400)) + 1,
	idx402 = idx401 + sizeof(STATUS(401)) + 1,
	idx403 = idx402 + sizeof(STATUS(402)) + 1,
	idx404 = idx403 + sizeof(STATUS(403)) + 1,
	idx405 = idx404 + sizeof(STATUS(404)) + 1,
	idx406 = idx405 + sizeof(STATUS(405)) + 1,
	idx407 = idx406 + sizeof(STATUS(406)) + 1,
	idx408 = idx407 + sizeof(STATUS(407)) + 1,
	idx409 = idx408 + sizeof(STATUS(408)) + 1,
	idx410 = idx409 + sizeof(STATUS(409)) + 1,
	idx411 = idx410 + sizeof(STATUS(410)) + 1,
	idx412 = idx411 + sizeof(STATUS(411)) + 1,
	idx413 = idx412 + sizeof(STATUS(412)) + 1,
	idx414 = idx413 + sizeof(STATUS(413)) + 1,
	idx415 = idx414 + sizeof(STATUS(414)) + 1,
	idx416 = idx415 + sizeof(STATUS(415)) + 1,
	idx417 = idx416 + sizeof(STATUS(416)) + 1,
	idx418 = idx417 + sizeof(STATUS(417)) + 1,
	idx421 = idx418 + sizeof(STATUS(418)) + 1,
	idx422 = idx421 + sizeof(STATUS(421)) + 1,
	idx423 = idx422 + sizeof(STATUS(422)) + 1,
	idx424 = idx423 + sizeof(STATUS(423)) + 1,
	idx425 = idx424 + sizeof(STATUS(424)) + 1,
	idx426 = idx425 + sizeof(STATUS(425)) + 1,
	idx428 = idx426 + sizeof(STATUS(426)) + 1,
	idx429 = idx428 + sizeof(STATUS(428)) + 1,
	idx431 = idx429 + sizeof(STATUS(429)) + 1,
	idx500 = idx431 + sizeof(STATUS(431)) + 1,
	idx501 = idx500 + sizeof(STATUS(500)) + 1,
	idx502 = idx501 + sizeof(STATUS(501)) + 1,
	idx503 = idx502 + sizeof(STATUS(502)) + 1,
	idx504 = idx503 + sizeof(STATUS(503)) + 1,
	idx505 = idx504 + sizeof(STATUS(504)) + 1,
	idx506 = idx505 + sizeof(STATUS(505)) + 1,
	idx507 = idx506 + sizeof(STATUS(506)) + 1,
	idx508 = idx507 + sizeof(STATUS(507)) + 1,
	idx510 = idx508 + sizeof(STATUS(508)) + 1,
	idx511 = idx510 + sizeof(STATUS(510)) + 1
};

static const u16 offsets[] =
{
	idx100, idx101, idx102, idx103, idx104,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	idx201, idx201, idx202, idx203, idx204, idx205, idx206, idx207, idx208,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, idx226, 1, 1, 1, 1, 1,
	idx301, idx301, idx302, idx303, idx304, idx305, idx306, idx307, idx308,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	idx401, idx401, idx402, idx403, idx404, idx405, idx406, idx407, idx408, idx409,
	idx411, idx411, idx412, idx413, idx414, idx415, idx416, idx417, idx418,	1, 1,
	idx421, idx422, idx423, idx424, idx425, idx426, 1, idx428, idx429, 1, idx431,
	idx501, idx501, idx502, idx503, idx504, idx505, idx506, idx507, idx508,
	1, idx511, idx511, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
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

// C++98
class Status {
public:
	u16 index;

	Status(usize number) {
		usize div = number / 100 - 100;
		usize rem = number % 100;

		index = (number < 100 || number >= 512 || rem >= 32) 
			? 1 : offsets[div * 32 + rem];
	}

	const char *c_str() {
		return s_statusCodes + index;
	}

	usize size() {
		return (usize)s_statusCodes[index - 1];
	}
};
}
