#pragma once

#include "core.hpp"
#include "Arena.hpp"

namespace HTTP {

class StringView32 {
public:
	u32 offset;
	u32 length;

	StringView32() : offset(0), length(0) {}
	StringView32(u32 length, u32 offset) : offset(offset), length(length) {}

	const char *c_str() const {
		return (const char*)Arena::get_ptr(offset);
	}

	char *c_str_mut() const {
		return (char*)Arena::get_ptr(offset);
	}

	template <usize size>
	bool operator==(const char (&literal)[size]) const {
		return length == size - 1 && MEMCMP_INLINE(c_str(), literal) == 0;
	}
};

class StringView {
public:
	char *ptr;
	usize length;

	StringView() : ptr(0), length(0) {}
	StringView(char* newPtr, u32 length) : ptr(newPtr), length(length) {}

	template <usize size>
	bool operator==(const char (&literal)[size]) const {
		return length == size - 1 && MEMCMP_INLINE(ptr, literal) == 0;
	}
};

}
