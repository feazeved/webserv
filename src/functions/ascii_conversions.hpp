#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"

namespace fn {
// 

FN_ATTR(always_inline, pure) static
Span itoa10(usize number, char* buffer, usize bufferSize) {
	ASSERT(bufferSize >= 24, "Buffer isn't big enough for itoa");
	char* ptr = buffer + 24;
	ptr += 24;
	*ptr = 0;
	char *const optr = ptr;
	do {
		*--ptr = (char)((number % 10) + '0');
		number /= 10;
	}	while (number != 0);
	Span result = {ptr, (usize)(optr - ptr)};
	return result;
}

FN_ATTR(always_inline, pure) static
Span itoa16(usize number, char* buffer, usize bufferSize) {
	static const char digits[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	ASSERT(bufferSize >= 24, "Buffer isn't big enough for itoa");
	char* ptr = buffer + 24;
	ptr += 24;
	*ptr = 0;
	char *const optr = ptr;
	do {
		*--ptr = digits[(number % 16)];
		number /= 16;
	}	while (number != 0);
	Span result = {ptr, (usize)(optr - ptr)};
	return result;
}

FN_ATTR(always_inline, pure) static
usize qnumlen10(const char* src) {
	char buffer[32] = {};
	MEMCPY_INLINE(buffer, src, 24);

	char* ptr = buffer;
	while (*ptr >= '0' && *ptr <= '9')
		ptr++;

	const usize length = (usize)(ptr - buffer);
	return length == 16 ? SIZE_MAX : length;
}

FN_ATTR(always_inline, pure) static
usize qnumlen16(const char* src) {
	char buffer[32] = {};
	MEMCPY_INLINE(buffer, src, 24);

	char* ptr = buffer;
	while (g_asciiLut[(u8)*ptr] <= ASCII_HEX)
		ptr++;

	const usize length = (usize)(ptr - buffer);
	return length == 16 ? SIZE_MAX : length;
}

FN_ATTR(always_inline, pure) static
usize strtol10(const char* src) {
	char buffer[32] = {};
	MEMCPY_INLINE(buffer, src, 24);
	usize value;
	usize digit;

	char* ptr = buffer;
	while ((digit = (usize)(*ptr - '0')) <= 9) {
		value = value * 10 + digit;
		ptr++;
	}
	const usize length = (usize)(ptr - buffer);
	return length <= 19 ? value : SIZE_MAX;
}

FN_ATTR(always_inline, pure) static
usize strtol16(const char* src) {
	char buffer[32] = {};
	MEMCPY_INLINE(buffer, src, 16);
	usize value;
	usize digit;

	char* ptr = buffer;
	while ((digit = (usize) g_asciiLut[(u8)*ptr]) <= ASCII_HEX) {
		value = value * 16 + digit;
		ptr++;
	}
	const usize length = (usize)(ptr - buffer);
	return length <= 15 ? value : SIZE_MAX;
}

// FN_ATTR(always_inline, pure) static
// usize strtol10(const char* src, usize &length) {
// 	char buffer[32] = {};
// 	MEMCPY_INLINE(buffer, src, 24);
// 	usize value;
// 	usize digit;

// 	char* ptr = buffer;
// 	while ((digit = (usize)(*ptr - '0')) <= 9) {
// 		value = value * 10 + digit;
// 		ptr++;
// 	}
// 	length = (usize)(ptr - buffer);
// 	return length <= 19 ? value : SIZE_MAX;
// }

// FN_ATTR(always_inline, pure) static
// usize strtol16(const char* src, usize &length) {
// 	char buffer[32] = {};
// 	MEMCPY_INLINE(buffer, src, 16);
// 	usize value;
// 	usize digit;

// 	char* ptr = buffer;
// 	while ((digit = (usize) g_asciiLut[(u8)*ptr]) <= ASCII_HEX) {
// 		value = value * 16 + digit;
// 		ptr++;
// 	}
// 	length = (usize)(ptr - buffer);
// 	return length <= 15 ? value : SIZE_MAX;
// }

}