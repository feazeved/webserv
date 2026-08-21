#pragma once
#include "core.hpp"
#include "Arena.hpp"

template <typename Type>
class Array {
public:
	u32 offset;
	u32 count;

	Type& operator[] (usize index) {
		Array* ptr = (Array*)(Arena::data + offset);
		return ptr[index]; 
	}
};
