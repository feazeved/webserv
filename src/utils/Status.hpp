#pragma once
#include "core.hpp"
#include "core_builtins.ipp"
#include "status_codes.hpp"
#include "StringView.hpp"

namespace HTTP {

/*
	Status stores a u16 offset to a status string in Arena::poolB.  Every
	status record is length-prefixed and NUL-terminated.  Error records append
	a length-prefixed, NUL-terminated default page immediately after the status.

	Index 1 is unset and index 2 is invalid.  Both resolve to empty strings.
*/
class Status {
public:
	static const usize errorPageCount = 32 + 12;
	static const usize arenaOffset = sizeof(Arena::pool.A);
	static char *const startPtr;

	u16 index;

	#pragma push_macro("ADDS")
	#pragma push_macro("ADDP")
	#undef ADDS
	#undef ADDP
	#define ADDS(code) (i##code + sizeof(HTTP_STATUS(code)) + 1)
	#define ADDP(code) \
		(i##code + sizeof(HTTP_STATUS(code)) \
		+ sizeof(HTTP_STATUS_DEFAULT_PAGE(code)) + 2)
	enum Code
	{
		i000 = 1, ixxx = 2, i100 = 9,       i101 = ADDS(100), i102 = ADDS(101),
		i103 = ADDS(102), i104 = ADDS(103), i200 = ADDS(104), i201 = ADDS(200),
		i202 = ADDS(201), i203 = ADDS(202), i204 = ADDS(203), i205 = ADDS(204),
		i206 = ADDS(205), i207 = ADDS(206), i208 = ADDS(207), i226 = ADDS(208),
		i300 = ADDS(226), i301 = ADDS(300), i302 = ADDS(301), i303 = ADDS(302),
		i304 = ADDS(303), i305 = ADDS(304), i306 = ADDS(305), i307 = ADDS(306),
		i308 = ADDS(307), i400 = ADDS(308), i401 = ADDP(400), i402 = ADDP(401),
		i403 = ADDP(402), i404 = ADDP(403), i405 = ADDP(404), i406 = ADDP(405),
		i407 = ADDP(406), i408 = ADDP(407), i409 = ADDP(408), i410 = ADDP(409),
		i411 = ADDP(410), i412 = ADDP(411), i413 = ADDP(412), i414 = ADDP(413),
		i415 = ADDP(414), i416 = ADDP(415), i417 = ADDP(416), i418 = ADDP(417),
		i421 = ADDP(418), i422 = ADDP(421), i423 = ADDP(422), i424 = ADDP(423),
		i425 = ADDP(424), i426 = ADDP(425), i428 = ADDP(426), i429 = ADDP(428),
		i431 = ADDP(429), i500 = ADDP(431), i501 = ADDP(500), i502 = ADDP(501),
		i503 = ADDP(502), i504 = ADDP(503), i505 = ADDP(504), i506 = ADDP(505),
		i507 = ADDP(506), i508 = ADDP(507), i510 = ADDP(508), i511 = ADDP(510)
	};
	#pragma pop_macro("ADDP")
	#pragma pop_macro("ADDS")

	ALWAYS_INLINE
	static u16 s_index(usize div, usize rem) {
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
		if (rem >= 32)
			return (u16)ixxx;
		return s_offsets[div * 32 + rem];
	}

	ALWAYS_INLINE
	static u16 s_num_to_code(usize number) {
		if (number - 100 >= 500)
			return ixxx;
		const usize div = number / 100;
		return s_index(div - 1, number - div * 100);
	}

	ALWAYS_INLINE
	static u16 s_str_to_code(const char *str) {
		if (str[0] < '1' || str[0] > '5' ||
			str[1] < '0' || str[1] > '9' ||
			str[2] < '0' || str[2] > '9')
				return ixxx;
		
		const usize div = (usize)(str[0] - '1');
		const usize rem = 10 * (usize)(str[1] - '0') + (usize)(str[2] - '0');
		return s_index(div, rem);
	}

	ALWAYS_INLINE
	static usize s_code_to_index(Code code) {
		char* ptr = startPtr + (usize) code;
		usize first = (u8)(ptr[0] - '0');
		usize second = (u8)(ptr[1] - '0');
		usize third = (u8)(ptr[2] - '0');
		return first * 32 + second * 10 + third;
	}

	ALWAYS_INLINE
	usize get_page_index() {
		if (index < Status::i400)
			return SIZE_MAX;
		return s_code_to_index((Code)index) - (32ul * 3);
	}

	ALWAYS_INLINE
	StringView status_str() const {
		StringView result;
		result.ptr = startPtr + (usize) index;
		result.length = (u8) result.ptr[-1];
		return result;
	}

	ALWAYS_INLINE
	StringView error_str() const {
		StringView result;
		result.ptr = startPtr + (usize) index;
		result.length = (u8) result.ptr[-1];
	
		result.ptr += result.length + 2;
		result.length = (u8)result.ptr[-1];
		return result;
	}

	ALWAYS_INLINE
	static StringView32 s_error_str(usize number) {
		usize offset = s_index(number >= 500, number % 32);
		StringView tmp;
		tmp.ptr = startPtr + (usize) offset;
		tmp.length = (u8) tmp.ptr[-1];
		tmp.ptr += tmp.length + 2;
		tmp.length = (u8)tmp.ptr[-1];
	
		StringView32 result;
		result.offset = (u32)(tmp.ptr - startPtr) + arenaOffset;
		result.length = tmp.length;
		return result;
	}

	ALWAYS_INLINE
	void reset() {
		index = i000;
	}

	// Utilities
	ALWAYS_INLINE
	usize number() const {
		const char *str = status_str().ptr;
		usize number = 100 * (usize)(str[0] - '0');
		number += 10 * (usize)(str[1] - '0');
		number += (usize)(str[2] - '0');
		return number;
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

	// Constructors and Overloads
	ALWAYS_INLINE
	Status() : index(i000) {}

	ALWAYS_INLINE
	Status(Code code) : index((u16)code) {}

	ALWAYS_INLINE
	explicit Status(usize number) : index(s_num_to_code(number)) {}

	ALWAYS_INLINE
	explicit Status(const char *str) : index(s_str_to_code(str)) {}

	ALWAYS_INLINE
	Status& operator=(Code code) {
		index = (u16)code;
		return *this;
	}

	ALWAYS_INLINE
	Status& operator=(usize number) {
		index = s_num_to_code(number);
		return *this;
	}

	ALWAYS_INLINE
	Status& operator=(const char *str) {
		index = s_str_to_code(str);
		return *this;
	}

	ALWAYS_INLINE
	Status& operator=(const u8 *str) {
		return *this = (const char*)str;
	}

	ALWAYS_INLINE
	bool operator==(Code code) const {
		return index == (u16)code;
	}

	ALWAYS_INLINE
	bool operator!=(Code code) const {
		return index != (u16)code;
	}
};

#ifdef MAIN_FILE
	char *const Status::startPtr = (char*) Arena::poolB;
#endif

STATIC_ASSERT(sizeof(HTTP_STATUS_STRINGS) <= UINT16_MAX);
STATIC_ASSERT(sizeof(Status) == sizeof(u16));

}
