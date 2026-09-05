#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"

namespace fn {
// 

FN_ATTR(pure) static inline
usize html_encoded_size(const char* src, usize length) {
	static const u8 growthLut[6] = {4, 4, 5, 3, 3, 0};
	usize result = length;

	for (usize index = 0; index < length; index++) {
		u8 lutIndex = g_asciiLut[(u8)src[index]] - ASCII_HTML_VALID;
		lutIndex = MIN(5, lutIndex);
		result += growthLut[lutIndex];
	}
	return result;
}

FN_ATTR(always_inline) static inline
Span itoa10(usize number, char* buffer, usize bufferSize) {
	ASSERT(bufferSize >= 20, "Buffer isn't big enough for itoa");
	char* ptr = buffer + bufferSize;
	ptr += bufferSize;
	*ptr = 0;
	char *const optr = ptr;
	do {
		*--ptr = (char)((number % 10) + '0');
		number /= 10;
	}	while (number != 0);
	Span result = {ptr, (usize)(optr - ptr)};
	return result;
}

FN_ATTR(always_inline) static inline
Span itoa16(usize number, char* buffer, usize bufferSize) {
	static const char digits[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	ASSERT(bufferSize >= 16, "Buffer isn't big enough for itoa");
	char* ptr = buffer + bufferSize;
	ptr += bufferSize;
	*ptr = 0;
	char *const optr = ptr;
	do {
		*--ptr = digits[(number % 16)];
		number /= 16;
	}	while (number != 0);
	Span result = {ptr, (usize)(optr - ptr)};
	return result;
}

FN_ATTR(always_inline, pure) static inline
usize strtol10(const char* src, usize minLength = 1, usize maxLength = 19) {
	char buffer[32] = {};
	MEMCPY_INLINE(buffer, src, 24);
	usize value = 0;
	usize digit;

	char* ptr = buffer;
	while ((digit = (usize)(*ptr - '0')) <= 9) {
		value = value * 10 + digit;
		ptr++;
	}
	const usize length = (usize)(ptr - buffer);
	return length >= minLength && length <= maxLength ? value : SIZE_MAX;
}

FN_ATTR(always_inline, pure) static inline
usize strtol16(const char* src, usize minLength = 1, usize maxLength = 15) {
	char buffer[32] = {};
	MEMCPY_INLINE(buffer, src, 24);
	usize value = 0;
	usize digit;

	char* ptr = buffer;
	while ((digit = (usize) g_asciiLut[(u8)*ptr]) <= ASCII_HEX) {
		value = value * 16 + digit;
		ptr++;
	}
	const usize length = (usize)(ptr - buffer);
	return length >= minLength && length <= maxLength ? value : SIZE_MAX;
}
}
