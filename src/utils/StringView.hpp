#pragma once
#include "core.hpp"
#include "Arena.hpp"
#include "Span.hpp"

class StringView32 {
public:
	u32 offset;
	u32 length;

	StringView32() : offset(0), length(0) {}
	StringView32(u32 length, u32 offset) : offset(offset), length(length) {}

	const char *kptr() const {
		return (const char*)Arena::mptr(offset);
	}

	char *mptr() const {
		return (char*)Arena::mptr(offset);
	}

	Span extract() {
		Span result;
		result.ptr = (char*)Arena::mptr(offset);
		result.length = length;
		return result;
	}

	template <usize size>
	bool operator==(const char (&literal)[size]) const {
		return length == size - 1 && STRCMP(kptr(), literal) == 0;
	}
};

