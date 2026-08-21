#pragma once

#include "core.hpp"
#include "Arena.hpp"

namespace HTTP {

class StringView {
public:
	u32 offset;
	u32 length;

	StringView() : length(0), offset(0) {}
	StringView(u32 length, u32 offset) : length(length), offset(offset) {}

	const char *get() const {
		return (const char*)Arena::data + offset;
	}

	template <usize size>
	bool operator==(const char (&literal)[size]) const {
		return length == size - 1 && MEMCMP_INLINE(get(), literal) == 0;
	}
};

}
