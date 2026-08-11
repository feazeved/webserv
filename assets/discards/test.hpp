#include "core.hpp"

template <usize bufferSize>
struct SmallString{
	u8 size;
	char data[bufferSize];

	template <usize N>
	SmallString(const char (&string)[N]) : data(), size(N - 1) 
	{
		MEMCPY_INLINE(data, string, N - 1);
	}
};

static inline
isize s_match2(const char *ptr, const char *end){
	static const char ltable[][32] = 
		{"status", "location", "transfer-encoding", "content-length"};		

	const char *optr = ptr;
	while (ptr < end && *ptr != ':')
		ptr++;
	usize length = (usize)(ptr - optr);
	if (length >= sizeof(ltable) || *ptr != ':')
		return (*ptr != ':') ? -1 : 0;
	ptr++;

	char buffer[64];

	MEMCPY_INLINE(buffer, ptr, 32);
	for (usize i = 0; i < 32; i++)
		buffer[i] |= 32;
	MEMSET_INLINE(buffer + length, 0, 32);

	for (usize i = 0; i < 32; i++) {
		if (MEMCMP(ltable[i], buffer, 32) == 0)
			return (isize)i + 1;
	}
	return 0;	
}

isize s_match(const char *ptr, const char *end) {
	static const char ltable[][5] = {
		{'h','t','m','l'},
		{'h','t','m'},
		{'c','s','s'},
		{'j','s','o','n'},
		{'j','s'},
		{'p','n','g'},
		{'j','p','g'},
		{'j','p','e','g'},
		{'g','i','f'},
		{'t','x','t'}
	};

	const char *optr = ptr;
	while (ptr < end && *ptr != ':')
		ptr++;
	usize length = (usize)(ptr - optr);
	if (length >= sizeof(ltable) || *ptr != ':')
		return (*ptr != ':') ? -1 : 0;
	ptr++;

	char buffer[64];

	MEMCPY_INLINE(buffer, ptr, 32);
	for (usize i = 0; i < 32; i++)
		buffer[i] |= 32;

	for (usize i = 0; i < 32; i++) {
		if (MEMCMP(ltable[i] + 1, buffer, (usize)ltable[i][0]) == 0)
			return (isize)i + 1;
	}
	return 0;
}
