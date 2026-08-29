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

	operator char*() {
		return ptr;
	}
};

struct Span32 {
	u32 index;
	u32 length;
};

struct Span16 {
	u16 index;
	u16 length;

	Span extract(char* ptr) {
		Span result;
		result.ptr = ptr + index;
		result.length = length;
		return result;
	}
};
