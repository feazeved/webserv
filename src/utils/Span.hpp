#pragma once
#include "core.hpp"

struct Span {
	char* ptr;
	usize size;

	char* end() const {
		return ptr + size;
	}

	template <usize length>
	bool operator==(const char (&literal)[length]) const {
		return size == length - 1 && STRCMP(ptr, literal) == 0;
	}

	template <usize length>
	static Span create(const char (&literal)[length]) {
		Span newSpan = {(char*)literal, length - 1};
		return newSpan;
	}

	static Span create(char* srcPtr, usize srcSize) {
		Span newSpan = {srcPtr, srcSize};
		return newSpan;
	}

	operator char*() const {
		return ptr;
	}
};

// struct Span32 {
// 	u32 index;
// 	u32 length;
// };

// A ZPtr could be an object that only contains its index
// The length is encoded in the location, so extracting is ptr = *zptr + 8, length = *zptr[8]
// But this doesn't save memory. It just shifts the size to the arena rather than the array

struct Span16 {
	u16 index;
	u16 size;
};

// Span extract(char* ptr) const {
// 	Span result;
// 	result.ptr = ptr + index;
// 	result.size = length;
// 	return result;
// }
