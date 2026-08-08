#pragma once
#include "core.hpp"
#include "core_builtins.ipp"

namespace HTTP {

#define HTTP_STATUS_100 "100 Continue"
#define HTTP_STATUS_101 "101 Switching Protocols"
#define HTTP_STATUS_102 "102 Processing"
#define HTTP_STATUS_103 "103 Early Hints"
#define HTTP_STATUS_104 "104 Upload Resumption Supported"
#define HTTP_STATUS_200 "200 OK"
#define HTTP_STATUS_201 "201 Created"
#define HTTP_STATUS_202 "202 Accepted"
#define HTTP_STATUS_203 "203 Non-Authoritative Information"
#define HTTP_STATUS_204 "204 No Content"
#define HTTP_STATUS_205 "205 Reset Content"
#define HTTP_STATUS_206 "206 Partial Content"
#define HTTP_STATUS_207 "207 Multi-Status"
#define HTTP_STATUS_208 "208 Already Reported"
#define HTTP_STATUS_226 "226 IM Used"
#define HTTP_STATUS_300 "300 Multiple Choices"
#define HTTP_STATUS_301 "301 Moved Permanently"
#define HTTP_STATUS_302 "302 Found"
#define HTTP_STATUS_303 "303 See Other"
#define HTTP_STATUS_304 "304 Not Modified"
#define HTTP_STATUS_305 "305 Use Proxy"
#define HTTP_STATUS_306 "306 (Unused)"
#define HTTP_STATUS_307 "307 Temporary Redirect"
#define HTTP_STATUS_308 "308 Permanent Redirect"
#define HTTP_STATUS_400 "400 Bad Request"
#define HTTP_STATUS_401 "401 Unauthorized"
#define HTTP_STATUS_402 "402 Payment Required"
#define HTTP_STATUS_403 "403 Forbidden"
#define HTTP_STATUS_404 "404 Not Found"
#define HTTP_STATUS_405 "405 Method Not Allowed"
#define HTTP_STATUS_406 "406 Not Acceptable"
#define HTTP_STATUS_407 "407 Proxy Authentication Required"
#define HTTP_STATUS_408 "408 Request Timeout"
#define HTTP_STATUS_409 "409 Conflict"
#define HTTP_STATUS_410 "410 Gone"
#define HTTP_STATUS_411 "411 Length Required"
#define HTTP_STATUS_412 "412 Precondition Failed"
#define HTTP_STATUS_413 "413 Content Too Large"
#define HTTP_STATUS_414 "414 URI Too Long"
#define HTTP_STATUS_415 "415 Unsupported Media Type"
#define HTTP_STATUS_416 "416 Range Not Satisfiable"
#define HTTP_STATUS_417 "417 Expectation Failed"
#define HTTP_STATUS_418 "418 (Unused)"
#define HTTP_STATUS_421 "421 Misdirected Request"
#define HTTP_STATUS_422 "422 Unprocessable Content"
#define HTTP_STATUS_423 "423 Locked"
#define HTTP_STATUS_424 "424 Failed Dependency"
#define HTTP_STATUS_425 "425 Too Early"
#define HTTP_STATUS_426 "426 Upgrade Required"
#define HTTP_STATUS_428 "428 Precondition Required"
#define HTTP_STATUS_429 "429 Too Many Requests"
#define HTTP_STATUS_431 "431 Request Header Fields Too Large"
#define HTTP_STATUS_451 "451 Unavailable For Legal Reasons"
#define HTTP_STATUS_500 "500 Internal Server Error"
#define HTTP_STATUS_501 "501 Not Implemented"
#define HTTP_STATUS_502 "502 Bad Gateway"
#define HTTP_STATUS_503 "503 Service Unavailable"
#define HTTP_STATUS_504 "504 Gateway Timeout"
#define HTTP_STATUS_505 "505 HTTP Version Not Supported"
#define HTTP_STATUS_506 "506 Variant Also Negotiates"
#define HTTP_STATUS_507 "507 Insufficient Storage"
#define HTTP_STATUS_508 "508 Loop Detected"
#define HTTP_STATUS_510 "510 Not Extended"
#define HTTP_STATUS_511 "511 Network Authentication Required"
#define HTTP_STATUS(code) HTTP_STATUS_##code

/*
	This class indexes HTTP Statuses in a lookup-table for O1 string retrieval.
	The strings are stored linearly in memory, like so [length1][string1][\0][length2][string2][\0]

	If the index stored is 1, it means the index was not set
	If the index stored is 2, it means the status is invalid

	Example Usage:
		Get a compile-time string: 
			Status::c_str(Status::i100)
		Read a string/number/enum and store it as status:
			Status status; status = (str/number/enum);
*/
class Status {
public:
	u16 index;

	#pragma push_macro("ADD")
	#undef ADD
	#define ADD(code) (i##code + sizeof(HTTP_STATUS(code)) + 1)
	enum Code
	{
		i000 = 1,        i100 = 3,        i101 = ADD(100), i102 = ADD(101),
		i103 = ADD(102), i104 = ADD(103), i200 = ADD(104), i201 = ADD(200),
		i202 = ADD(201), i203 = ADD(202), i204 = ADD(203), i205 = ADD(204),
		i206 = ADD(205), i207 = ADD(206), i208 = ADD(207), i226 = ADD(208),
		i300 = ADD(226), i301 = ADD(300), i302 = ADD(301), i303 = ADD(302),
		i304 = ADD(303), i305 = ADD(304), i306 = ADD(305), i307 = ADD(306),
		i308 = ADD(307), i400 = ADD(308), i401 = ADD(400), i402 = ADD(401),
		i403 = ADD(402), i404 = ADD(403), i405 = ADD(404), i406 = ADD(405),
		i407 = ADD(406), i408 = ADD(407), i409 = ADD(408), i410 = ADD(409),
		i411 = ADD(410), i412 = ADD(411), i413 = ADD(412), i414 = ADD(413),
		i415 = ADD(414), i416 = ADD(415), i417 = ADD(416), i418 = ADD(417),
		i421 = ADD(418), i422 = ADD(421), i423 = ADD(422), i424 = ADD(423),
		i425 = ADD(424), i426 = ADD(425), i428 = ADD(426), i429 = ADD(428),
		i431 = ADD(429), i500 = ADD(431), i501 = ADD(500), i502 = ADD(501),
		i503 = ADD(502), i504 = ADD(503), i505 = ADD(504), i506 = ADD(505),
		i507 = ADD(506), i508 = ADD(507), i510 = ADD(508), i511 = ADD(510)
	};
	#pragma pop_macro("ADD")

	ALWAYS_INLINE
	static u16 s_index(usize div, usize rem) {
		static const u16 s_offsets[160] = {
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
		return (rem >= 32) ? 2 : s_offsets[div * 32 + rem];
	}

	ALWAYS_INLINE
	static const char *s_strings(usize argIndex) {
		static const char s_statusCodes[] = "\0\0\x0C"
		HTTP_STATUS(100) "\0\x17" HTTP_STATUS(101) "\0\x0E" HTTP_STATUS(102) "\0\x0F"
		HTTP_STATUS(103) "\0\x1F" HTTP_STATUS(104) "\0\x06" HTTP_STATUS(200) "\0\x0B"
		HTTP_STATUS(201) "\0\x0C" HTTP_STATUS(202) "\0\x21" HTTP_STATUS(203) "\0\x0E"
		HTTP_STATUS(204) "\0\x11" HTTP_STATUS(205) "\0\x13" HTTP_STATUS(206) "\0\x10"
		HTTP_STATUS(207) "\0\x14" HTTP_STATUS(208) "\0\x0B" HTTP_STATUS(226) "\0\x14"
		HTTP_STATUS(300) "\0\x15" HTTP_STATUS(301) "\0\x09" HTTP_STATUS(302) "\0\x0D"
		HTTP_STATUS(303) "\0\x10" HTTP_STATUS(304) "\0\x0D" HTTP_STATUS(305) "\0\x0C"
		HTTP_STATUS(306) "\0\x16" HTTP_STATUS(307) "\0\x16" HTTP_STATUS(308) "\0\x0F"
		HTTP_STATUS(400) "\0\x10" HTTP_STATUS(401) "\0\x14" HTTP_STATUS(402) "\0\x0D"
		HTTP_STATUS(403) "\0\x0D" HTTP_STATUS(404) "\0\x16" HTTP_STATUS(405) "\0\x12"
		HTTP_STATUS(406) "\0\x21" HTTP_STATUS(407) "\0\x13" HTTP_STATUS(408) "\0\x0C"
		HTTP_STATUS(409) "\0\x08" HTTP_STATUS(410) "\0\x13" HTTP_STATUS(411) "\0\x17"
		HTTP_STATUS(412) "\0\x15" HTTP_STATUS(413) "\0\x10" HTTP_STATUS(414) "\0\x1A"
		HTTP_STATUS(415) "\0\x19" HTTP_STATUS(416) "\0\x16" HTTP_STATUS(417) "\0\x0C"
		HTTP_STATUS(418) "\0\x17" HTTP_STATUS(421) "\0\x19" HTTP_STATUS(422) "\0\x0A"
		HTTP_STATUS(423) "\0\x15" HTTP_STATUS(424) "\0\x0D" HTTP_STATUS(425) "\0\x14"
		HTTP_STATUS(426) "\0\x19" HTTP_STATUS(428) "\0\x15" HTTP_STATUS(429) "\0\x23"
		HTTP_STATUS(431) "\0\x19" HTTP_STATUS(500) "\0\x13" HTTP_STATUS(501) "\0\x0F"
		HTTP_STATUS(502) "\0\x17" HTTP_STATUS(503) "\0\x13" HTTP_STATUS(504) "\0\x1E"
		HTTP_STATUS(505) "\0\x1B" HTTP_STATUS(506) "\0\x18" HTTP_STATUS(507) "\0\x11"
		HTTP_STATUS(508) "\0\x10" HTTP_STATUS(510) "\0\x23" HTTP_STATUS(511) "\0";
		return s_statusCodes + argIndex;
	}

	Status() : index(1) {}

	Status& operator=(Code code) {
		index = code;
		return *this;
	}

	Status& operator=(usize number) {
		usize div = number / 100;
		usize rem = number - div * 100;
		index = (number - 100 >= 500) ? 2 : s_index(div - 1, rem);
		return *this;
	}

	Status& operator=(const char *str) {
		if (str[0] >= '1' && str[0] <= '5' && 
			str[1] >= '0' && str[1] <= '9' &&
			str[2] >= '0' && str[2] <= '9') {
			usize div = (usize)(str[0] - '1');
			usize rem = 10 * (usize)(str[1] - '0') + (usize)(str[2] - '0');
			index = s_index(div, rem);
		}
		else
			index = 2;
		return *this;
	}

	ALWAYS_INLINE
	static const char* c_str(Code value) {
		return s_strings(value);
	}

	ALWAYS_INLINE
	static usize size(Code value){
		return (usize) (s_strings(value)[-1]);
	}

	ALWAYS_INLINE
	const char *c_str() const {
		return s_strings(index);
	}

	ALWAYS_INLINE
	usize size() const {
		return (usize) (s_strings(index)[-1]);
	}

	ALWAYS_INLINE
	bool is_valid() const {
		return index > 2;
	}

	ALWAYS_INLINE
	bool is_error() const {
		return index >= i400;
	}

	ALWAYS_INLINE
	bool is_set() const {
		return index == 1;
	}
};
}
