#pragma once
#include "core.hpp"
#include "core_builtins.ipp"
#include "status_codes.hpp"
#include "StringView.hpp"

namespace HTTP {

/*
	This class indexes HTTP statuses in a lookup table for O(1) string retrieval.
	The strings are stored linearly in Arena::poolB as
	[length1][string1][\0][length2][string2][\0].

	Index 1 is unset and index 2 is invalid.  Both point at empty strings; valid
	indices point at the first byte of their status string.

	Example Usage:
		Get a compile-time string:
			Status::c_str(Status::i100)
		Read a string/number/enum and store it as status:
			Status status; status = (str/number/enum);
*/
class Status {
public:
	static const usize clientErrorCount = 32;
	static const usize serverErrorCount = 12;
	static const usize errorPageCount = clientErrorCount + serverErrorCount;

public:
	u16 index;

	#pragma push_macro("ADD")
	#undef ADD
	#define ADD(code) (i##code + sizeof(HTTP_STATUS(code)) + 1)
	enum Code
	{
		i000 = 1, ixxx = 2, i100 = 4,     i101 = ADD(100), i102 = ADD(101),
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

	#undef ADD
	#define ADD(code) (p##code + sizeof(HTTP_ERROR(code)))
	enum Page
	{
		p400 = sizeof(HTTP_STATUS_STRINGS) - 1,
		p401 = ADD(400), p402 = ADD(401), p403 = ADD(402),
		p404 = ADD(403), p405 = ADD(404), p406 = ADD(405),
		p407 = ADD(406), p408 = ADD(407), p409 = ADD(408),
		p410 = ADD(409), p411 = ADD(410), p412 = ADD(411),
		p413 = ADD(412), p414 = ADD(413), p415 = ADD(414),
		p416 = ADD(415), p417 = ADD(416), p418 = ADD(417),
		p419 = ADD(418), p420 = ADD(419), p421 = ADD(420),
		p422 = ADD(421), p423 = ADD(422), p424 = ADD(423),
		p425 = ADD(424), p426 = ADD(425), p427 = ADD(426),
		p428 = ADD(427), p429 = ADD(428), p430 = ADD(429),
		p431 = ADD(430), p500 = ADD(431), p501 = ADD(500),
		p502 = ADD(501), p503 = ADD(502), p504 = ADD(503),
		p505 = ADD(504), p506 = ADD(505), p507 = ADD(506),
		p508 = ADD(507), p509 = ADD(508), p510 = ADD(509),
		p511 = ADD(510), pEnd = ADD(511)
	};
	#pragma pop_macro("ADD")

	ALWAYS_INLINE
	static u16 s_code_index(usize div, usize rem) {
		static const u16 s_offsets[160] = {
			i100, i101, i102, i103, i104, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			i200, i201, i202, i203, i204, i205, i206, i207,
			i208, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, i226, ixxx, ixxx, ixxx, ixxx, ixxx,
			i300, i301, i302, i303, i304, i305, i306, i307,
			i308, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			i400, i401, i402, i403, i404, i405, i406, i407,
			i408, i409, i410, i411, i412, i413, i414, i415,
			i416, i417, i418, ixxx, ixxx, i421, i422, i423,
			i424, i425, i426, ixxx, i428, i429, ixxx, i431,
			i500, i501, i502, i503, i504, i505, i506, i507,
			i508, ixxx, i510, i511, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx,
			ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx, ixxx
		};
		return (rem >= 32) ? ixxx : s_offsets[div * 32 + rem];
	}

	ALWAYS_INLINE
	static usize s_page_idx(usize number) {
		if (number >= 400 && number <= 431)
			return (number - 400);
		else if (number >= 500 && number <= 511)
			return (clientErrorCount + number - 500);
		return SIZE_MAX;	
	}

	ALWAYS_INLINE
	static u16 s_page_index(usize number) {
		static const u16 s_offsets[errorPageCount] = {
			p400, p401, p402, p403, p404, p405, p406, p407,
			p408, p409, p410, p411, p412, p413, p414, p415,
			p416, p417, p418, p419, p420, p421, p422, p423,
			p424, p425, p426, p427, p428, p429, p430, p431,
			p500, p501, p502, p503, p504, p505, p506, p507,
			p508, p509, p510, p511
		};

		usize index = s_page_idx(number);
		if (index != SIZE_MAX)
			return s_offsets[index];
		return UINT16_MAX;
	}

	ALWAYS_INLINE
	static const char *s_strings(usize argIndex) {
		return (const char*)Arena::poolB + argIndex;
	}

	// TODO: This should go. The size should be encoded in the string, just like for status
	ALWAYS_INLINE
	static u16 s_page_size(usize number) {
		static const u16 s_offsets[errorPageCount] = {
			sizeof(HTTP_ERROR_400) - 1, sizeof(HTTP_ERROR_401) - 1,
			sizeof(HTTP_ERROR_402) - 1, sizeof(HTTP_ERROR_403) - 1,
			sizeof(HTTP_ERROR_404) - 1, sizeof(HTTP_ERROR_405) - 1,
			sizeof(HTTP_ERROR_406) - 1, sizeof(HTTP_ERROR_407) - 1,
			sizeof(HTTP_ERROR_408) - 1, sizeof(HTTP_ERROR_409) - 1,
			sizeof(HTTP_ERROR_410) - 1, sizeof(HTTP_ERROR_411) - 1,
			sizeof(HTTP_ERROR_412) - 1, sizeof(HTTP_ERROR_413) - 1,
			sizeof(HTTP_ERROR_414) - 1, sizeof(HTTP_ERROR_415) - 1,
			sizeof(HTTP_ERROR_416) - 1, sizeof(HTTP_ERROR_417) - 1,
			sizeof(HTTP_ERROR_418) - 1, sizeof(HTTP_ERROR_419) - 1,
			sizeof(HTTP_ERROR_420) - 1, sizeof(HTTP_ERROR_421) - 1,
			sizeof(HTTP_ERROR_422) - 1, sizeof(HTTP_ERROR_423) - 1,
			sizeof(HTTP_ERROR_424) - 1, sizeof(HTTP_ERROR_425) - 1,
			sizeof(HTTP_ERROR_426) - 1, sizeof(HTTP_ERROR_427) - 1,
			sizeof(HTTP_ERROR_428) - 1, sizeof(HTTP_ERROR_429) - 1,
			sizeof(HTTP_ERROR_430) - 1, sizeof(HTTP_ERROR_431) - 1,
			sizeof(HTTP_ERROR_500) - 1, sizeof(HTTP_ERROR_501) - 1,
			sizeof(HTTP_ERROR_502) - 1, sizeof(HTTP_ERROR_503) - 1,
			sizeof(HTTP_ERROR_504) - 1, sizeof(HTTP_ERROR_505) - 1,
			sizeof(HTTP_ERROR_506) - 1, sizeof(HTTP_ERROR_507) - 1,
			sizeof(HTTP_ERROR_508) - 1, sizeof(HTTP_ERROR_509) - 1,
			sizeof(HTTP_ERROR_510) - 1, sizeof(HTTP_ERROR_511) - 1
		};

		usize index = s_page_idx(number);
		if (index != SIZE_MAX)
			return s_offsets[index];
		return UINT16_MAX;
	}

	ALWAYS_INLINE
	Status() : index(i000) {}

	ALWAYS_INLINE
	Status(Code code) : index(code) {}

	// TODO: reverse this, constructors have the function and assign gets constructor
	ALWAYS_INLINE
	explicit Status(usize number) : index(i000) {
		*this = number;
	}

	ALWAYS_INLINE
	explicit Status(const char *str) : index(i000) {
		*this = str;
	}

	ALWAYS_INLINE
	Status& operator=(Code code) {
		index = code;
		return *this;
	}

	ALWAYS_INLINE
	Status& operator=(usize number) {
		usize div = number / 100;
		usize rem = number - div * 100;
		index = (number - 100 >= 500) ? ixxx : s_code_index(div - 1, rem);
		return *this;
	}

	ALWAYS_INLINE
	Status& operator=(const char *str) {
		if (str[0] >= '1' && str[0] <= '5'
			&& str[1] >= '0' && str[1] <= '9'
			&& str[2] >= '0' && str[2] <= '9') {
			const usize div = (usize)(str[0] - '1');
			const usize rem = 10 * (usize)(str[1] - '0') + (usize)(str[2] - '0');
			index = s_code_index(div, rem);
		}
		else
			index = ixxx;
		return *this;
	}

	ALWAYS_INLINE
	void reset() {
		index = i000;
	}

	ALWAYS_INLINE
	static const char* c_str(Code value) {
		return s_strings(value);
	}

	ALWAYS_INLINE
	static usize size(Code value) {
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

	// New
	ALWAYS_INLINE
	static usize error_code(usize slot) {
		if (slot < 32)
			return 400 + slot;
		if (slot < errorPageCount)
			return 500 + slot - clientErrorCount;
		return 0;
	}

	ALWAYS_INLINE
	static u16 default_error_index(usize number) {
		return s_page_index(number);
	}

	ALWAYS_INLINE
	static usize default_error_size(usize number) {
		return s_page_size(number);
	}

	// Don't know if this belongs here
	ALWAYS_INLINE
	static StringView32 default_error_page(usize number) {
		const usize pageSize = s_page_size(number);
		if (pageSize < 0)
			return StringView32();
		const usize pageIndex = s_page_index(number);
		return StringView32(pageSize, Arena::get_b_offset(pageIndex));
	}

	ALWAYS_INLINE
	bool operator==(Code code) const {
		return index == (u16)code;
	}

	ALWAYS_INLINE
	bool operator!=(Code code) const {
		return index != (u16)code;
	}

	ALWAYS_INLINE
	bool is_valid() const {
		return index > ixxx;
	}

	ALWAYS_INLINE
	bool is_informational() const {
		return index >= i100 && index < i200;
	}

	ALWAYS_INLINE
	bool is_success() const {
		return index >= i200 && index < i300;
	}

	ALWAYS_INLINE
	bool is_redirect() const {
		return index >= i300 && index < i400;
	}

	ALWAYS_INLINE
	bool is_client_error() const {
		return index >= i400 && index < i500;
	}

	ALWAYS_INLINE
	bool is_server_error() const {
		return index >= i500;
	}

	ALWAYS_INLINE
	bool is_error() const {
		return index >= i400;
	}

	ALWAYS_INLINE
	bool is_set() const {
		return index != i000;
	}
};

STATIC_ASSERT(Status::pEnd <= UINT16_MAX);
STATIC_ASSERT(Status::i511 <= UINT16_MAX);

}
