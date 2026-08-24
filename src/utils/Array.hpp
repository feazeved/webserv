#pragma once
#include "core.hpp"
#include "Arena.hpp"

template <typename Type>
class Array32 {
public:
	u32 offset;
	u32 count;

	Array32() : offset(0), count(0) {}

	bool alloc_a(u32 numElements) {
		usize totalSize = numElements * sizeof(Type);
		count = numElements;
		offset = Arena::alloc_a(totalSize);
		if (offset == UINT32_MAX)
			return true;
		return false;
	}

	bool alloc_b(u32 numElements) {
		usize totalSize = numElements * sizeof(Type);
		count = numElements;
		offset = Arena::alloc_b(totalSize);
		if (offset == UINT32_MAX)
			return true;
		return false;
	}

	Type& operator[] (usize index) {
		return ((Type*)(Arena::mptr(offset)))[index];
	}

	const Type& operator[] (usize index) const {
		return ((const Type*)(Arena::mptr(offset)))[index];
	}
};
