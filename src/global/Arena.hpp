#pragma once
#include <unistd.h>

#include "core.hpp"
#include "config.hpp"
#include "tables.hpp"
#include "Array.hpp"
#include "Span.hpp"

/*
	Arena has two pools, A that belongs to the connection pool and B
	that belongs to the config structure. Since parsing uses tokens
	and directives, they can be allocated in pool A, so their cost
	is effectively free because they can be safely overwritten

	Offsets use one logical address space: pool A starts at zero and pool B
	starts at sizeof(pool A).  The combined space stays well below 4 GB.
	Pool B begins with immutable status/error strings; allocations start at
	the next cache-line boundary.
*/

struct Arena {
	u8 *ptr;
	usize size, capacity;

	Arena (u8* srcPtr, usize srcLength) : ptr(srcPtr), size(0), capacity(srcLength) {

	}

	void clear() {
		size = 0;
	}

	u8* mptr(usize fileOffset) const {
		return ptr + fileOffset;
	}

	const u8* kptr(usize fileOffset) const {
		return ptr + fileOffset;
	}

	u32 alloc(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > capacity - size) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = size;
		size += bytes;
		return index;
	}

	// TODO: we could do different instatiation given a smaller range
	// For example, if size of elements is lower than 64k, indexes could be u16
	template <typename Type>
	Array<Type> alloc_array(usize numElements) {
		usize bytes = numElements * sizeof(Type);
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > capacity - size) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = size;
		size += bytes;
		return index;
	}

};
