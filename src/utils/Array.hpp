#pragma once
#include "core.hpp"
#include "Arena.hpp"

template <typename Type>
class Array32 {
public:
	u32 offset;
	u32 count;

	Array32() : offset(0), count(0) {}

	bool alloc(u32 numElements) {
		usize totalSize = numElements * sizeof(Type);
		count = numElements;
		offset = Arena::alloc_index(totalSize);
		if (offset == UINT32_MAX)
			return true;
		return false;
	}

	Type& operator[] (usize index) {
		return ((Type*)(Arena::data + offset))[index];
	}

	const Type& operator[] (usize index) const {
		return ((const Type*)(Arena::data + offset))[index];
	}
};
