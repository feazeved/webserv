#pragma once

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
#define HTTP_STATUS_418 "418 I'm a teapot"
#define HTTP_STATUS_421 "421 Misdirected Request"
#define HTTP_STATUS_422 "422 Unprocessable Content"
#define HTTP_STATUS_423 "423 Locked"
#define HTTP_STATUS_424 "424 Failed Dependency"
#define HTTP_STATUS_425 "425 Too Early"
#define HTTP_STATUS_426 "426 Upgrade Required"
#define HTTP_STATUS_428 "428 Precondition Required"
#define HTTP_STATUS_429 "429 Too Many Requests"
#define HTTP_STATUS_431 "431 Request Header Fields Too Large"
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
	Length-prefixed, NUL-terminated status strings.  Status stores a u16 byte
	offset into this table, so keep this order in sync with Status::Code.
*/
#define HTTP_STATUS_STRINGS \
	"\0\0\0\x0C" \
	HTTP_STATUS(100) "\0\x17" HTTP_STATUS(101) "\0\x0E" HTTP_STATUS(102) "\0\x0F" \
	HTTP_STATUS(103) "\0\x1F" HTTP_STATUS(104) "\0\x06" HTTP_STATUS(200) "\0\x0B" \
	HTTP_STATUS(201) "\0\x0C" HTTP_STATUS(202) "\0\x21" HTTP_STATUS(203) "\0\x0E" \
	HTTP_STATUS(204) "\0\x11" HTTP_STATUS(205) "\0\x13" HTTP_STATUS(206) "\0\x10" \
	HTTP_STATUS(207) "\0\x14" HTTP_STATUS(208) "\0\x0B" HTTP_STATUS(226) "\0\x14" \
	HTTP_STATUS(300) "\0\x15" HTTP_STATUS(301) "\0\x09" HTTP_STATUS(302) "\0\x0D" \
	HTTP_STATUS(303) "\0\x10" HTTP_STATUS(304) "\0\x0D" HTTP_STATUS(305) "\0\x0C" \
	HTTP_STATUS(306) "\0\x16" HTTP_STATUS(307) "\0\x16" HTTP_STATUS(308) "\0\x0F" \
	HTTP_STATUS(400) "\0\x10" HTTP_STATUS(401) "\0\x14" HTTP_STATUS(402) "\0\x0D" \
	HTTP_STATUS(403) "\0\x0D" HTTP_STATUS(404) "\0\x16" HTTP_STATUS(405) "\0\x12" \
	HTTP_STATUS(406) "\0\x21" HTTP_STATUS(407) "\0\x13" HTTP_STATUS(408) "\0\x0C" \
	HTTP_STATUS(409) "\0\x08" HTTP_STATUS(410) "\0\x13" HTTP_STATUS(411) "\0\x17" \
	HTTP_STATUS(412) "\0\x15" HTTP_STATUS(413) "\0\x10" HTTP_STATUS(414) "\0\x1A" \
	HTTP_STATUS(415) "\0\x19" HTTP_STATUS(416) "\0\x16" HTTP_STATUS(417) "\0\x10" \
	HTTP_STATUS(418) "\0\x17" HTTP_STATUS(421) "\0\x19" HTTP_STATUS(422) "\0\x0A" \
	HTTP_STATUS(423) "\0\x15" HTTP_STATUS(424) "\0\x0D" HTTP_STATUS(425) "\0\x14" \
	HTTP_STATUS(426) "\0\x19" HTTP_STATUS(428) "\0\x15" HTTP_STATUS(429) "\0\x23" \
	HTTP_STATUS(431) "\0\x21" HTTP_STATUS(500) "\0\x13" HTTP_STATUS(501) "\0\x0F" \
	HTTP_STATUS(502) "\0\x17" HTTP_STATUS(503) "\0\x13" HTTP_STATUS(504) "\0\x1E" \
	HTTP_STATUS(505) "\0\x1B" HTTP_STATUS(506) "\0\x18" HTTP_STATUS(507) "\0\x11" \
	HTTP_STATUS(508) "\0\x10" HTTP_STATUS(510) "\0\x23" HTTP_STATUS(511) "\0"

#define HTTP_ERROR_PAGE(code, reason) \
	"<!doctype html><meta charset=utf-8>" \
	"<title>" code " " reason "</title>" \
	"<style>body{font:16px sans-serif;text-align:center;padding:10vh}" \
	"h1{font-size:4em;margin:0}</style>" \
	"<h1>" code "</h1><p>" reason "</p></html>"

#define HTTP_ERROR_PAGE_MAX_SIZE 255

#define HTTP_ERROR_400 HTTP_ERROR_PAGE("400", "Bad Request")
#define HTTP_ERROR_401 HTTP_ERROR_PAGE("401", "Unauthorized")
#define HTTP_ERROR_402 HTTP_ERROR_PAGE("402", "Payment Required")
#define HTTP_ERROR_403 HTTP_ERROR_PAGE("403", "Forbidden")
#define HTTP_ERROR_404 HTTP_ERROR_PAGE("404", "Not Found")
#define HTTP_ERROR_405 HTTP_ERROR_PAGE("405", "Method Not Allowed")
#define HTTP_ERROR_406 HTTP_ERROR_PAGE("406", "Not Acceptable")
#define HTTP_ERROR_407 HTTP_ERROR_PAGE("407", "Proxy Authentication Required")
#define HTTP_ERROR_408 HTTP_ERROR_PAGE("408", "Request Timeout")
#define HTTP_ERROR_409 HTTP_ERROR_PAGE("409", "Conflict")
#define HTTP_ERROR_410 HTTP_ERROR_PAGE("410", "Gone")
#define HTTP_ERROR_411 HTTP_ERROR_PAGE("411", "Length Required")
#define HTTP_ERROR_412 HTTP_ERROR_PAGE("412", "Precondition Failed")
#define HTTP_ERROR_413 HTTP_ERROR_PAGE("413", "Content Too Large")
#define HTTP_ERROR_414 HTTP_ERROR_PAGE("414", "URI Too Long")
#define HTTP_ERROR_415 HTTP_ERROR_PAGE("415", "Unsupported Media Type")
#define HTTP_ERROR_416 HTTP_ERROR_PAGE("416", "Range Not Satisfiable")
#define HTTP_ERROR_417 HTTP_ERROR_PAGE("417", "Expectation Failed")
#define HTTP_ERROR_418 HTTP_ERROR_PAGE("418", "I'm a teapot")
#define HTTP_ERROR_419 HTTP_ERROR_PAGE("419", "Client Error")
#define HTTP_ERROR_420 HTTP_ERROR_PAGE("420", "Client Error")
#define HTTP_ERROR_421 HTTP_ERROR_PAGE("421", "Misdirected Request")
#define HTTP_ERROR_422 HTTP_ERROR_PAGE("422", "Unprocessable Content")
#define HTTP_ERROR_423 HTTP_ERROR_PAGE("423", "Locked")
#define HTTP_ERROR_424 HTTP_ERROR_PAGE("424", "Failed Dependency")
#define HTTP_ERROR_425 HTTP_ERROR_PAGE("425", "Too Early")
#define HTTP_ERROR_426 HTTP_ERROR_PAGE("426", "Upgrade Required")
#define HTTP_ERROR_427 HTTP_ERROR_PAGE("427", "Client Error")
#define HTTP_ERROR_428 HTTP_ERROR_PAGE("428", "Precondition Required")
#define HTTP_ERROR_429 HTTP_ERROR_PAGE("429", "Too Many Requests")
#define HTTP_ERROR_430 HTTP_ERROR_PAGE("430", "Client Error")
#define HTTP_ERROR_431 HTTP_ERROR_PAGE("431", "Request Header Fields Too Large")
#define HTTP_ERROR_500 HTTP_ERROR_PAGE("500", "Internal Server Error")
#define HTTP_ERROR_501 HTTP_ERROR_PAGE("501", "Not Implemented")
#define HTTP_ERROR_502 HTTP_ERROR_PAGE("502", "Bad Gateway")
#define HTTP_ERROR_503 HTTP_ERROR_PAGE("503", "Service Unavailable")
#define HTTP_ERROR_504 HTTP_ERROR_PAGE("504", "Gateway Timeout")
#define HTTP_ERROR_505 HTTP_ERROR_PAGE("505", "HTTP Version Not Supported")
#define HTTP_ERROR_506 HTTP_ERROR_PAGE("506", "Variant Also Negotiates")
#define HTTP_ERROR_507 HTTP_ERROR_PAGE("507", "Insufficient Storage")
#define HTTP_ERROR_508 HTTP_ERROR_PAGE("508", "Loop Detected")
#define HTTP_ERROR_509 HTTP_ERROR_PAGE("509", "Server Error")
#define HTTP_ERROR_510 HTTP_ERROR_PAGE("510", "Not Extended")
#define HTTP_ERROR_511 HTTP_ERROR_PAGE("511", "Network Authentication Required")
#define HTTP_ERROR(code) HTTP_ERROR_##code

#define HTTP_DEFAULT_ERROR_PAGES \
	HTTP_ERROR_400 "\0" HTTP_ERROR_401 "\0" HTTP_ERROR_402 "\0" HTTP_ERROR_403 "\0" \
	HTTP_ERROR_404 "\0" HTTP_ERROR_405 "\0" HTTP_ERROR_406 "\0" HTTP_ERROR_407 "\0" \
	HTTP_ERROR_408 "\0" HTTP_ERROR_409 "\0" HTTP_ERROR_410 "\0" HTTP_ERROR_411 "\0" \
	HTTP_ERROR_412 "\0" HTTP_ERROR_413 "\0" HTTP_ERROR_414 "\0" HTTP_ERROR_415 "\0" \
	HTTP_ERROR_416 "\0" HTTP_ERROR_417 "\0" HTTP_ERROR_418 "\0" HTTP_ERROR_419 "\0" \
	HTTP_ERROR_420 "\0" HTTP_ERROR_421 "\0" HTTP_ERROR_422 "\0" HTTP_ERROR_423 "\0" \
	HTTP_ERROR_424 "\0" HTTP_ERROR_425 "\0" HTTP_ERROR_426 "\0" HTTP_ERROR_427 "\0" \
	HTTP_ERROR_428 "\0" HTTP_ERROR_429 "\0" HTTP_ERROR_430 "\0" HTTP_ERROR_431 "\0" \
	HTTP_ERROR_500 "\0" HTTP_ERROR_501 "\0" HTTP_ERROR_502 "\0" HTTP_ERROR_503 "\0" \
	HTTP_ERROR_504 "\0" HTTP_ERROR_505 "\0" HTTP_ERROR_506 "\0" HTTP_ERROR_507 "\0" \
	HTTP_ERROR_508 "\0" HTTP_ERROR_509 "\0" HTTP_ERROR_510 "\0" HTTP_ERROR_511 "\0"

#define HTTP_ARENA_STATIC_STRINGS HTTP_STATUS_STRINGS HTTP_DEFAULT_ERROR_PAGES
