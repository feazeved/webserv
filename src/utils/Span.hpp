#pragma once
#include "core.hpp"

struct Span {
	char* ptr;
	usize length;

	char* end() const {
		return ptr + length;
	}

	template <usize size>
	bool operator==(const char (&literal)[size]) const {
		return length == size - 1 && STRCMP(ptr, literal) == 0;
	}

	operator char*() const {
		return ptr;
	}
};

struct Span32 {
	u32 index;
	u32 length;
};

// A ZPtr could be an object that only contains its index
// The length is encoded in the location, so extracting is ptr = *zptr + 8, length = *zptr[8]
// But this doesn't save memory. It just shifts the size to the arena rather than the array

struct Span16 {
	u16 index;
	u16 length;

	Span extract(char* ptr) const {
		Span result;
		result.ptr = ptr + index;
		result.length = length;
		return result;
	}
};
