#pragma once

#include "core.hpp"
#include <unistd.h>
#include "config.hpp"
#include "status_codes.hpp"

/*
	Arena has two pools, A that belongs to the connection pool and B
	that belongs to the config structure. Since parsing uses tokens
	and directives, they can be allocated in poolA, so their cost
	is effectively free because they can be safely overwritten

	There is no need to store different offsets, given that the memory
	layout is sequential and won't exceed 4 GB (not even close)
*/

class Arena {
private:
	Arena();
public:
	static u8 poolA[ARENA_SIZE] ALIGNED(4096);
	static u8 poolB[MAX_FILE_SIZE] ALIGNED(4096);
	static usize sizeA, sizeB;

	static void clear() {
		sizeA = 0;
		sizeB = 0;
	}

	static u8* get_ptr(usize fileOffset) {
		return poolA + fileOffset;
	}

	static u32 alloc_a(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > sizeof(poolA) - sizeA) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = sizeA;
		sizeA += bytes;
		return index;
	}

	static u32 alloc_b(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > sizeof(poolB) - sizeB) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = sizeB;
		sizeB += bytes;
		return index;
	}

};

STATIC_ASSERT(sizeof(HTTP_DEFAULT_ERROR_PAGES) <= sizeof(Arena::poolB));

#ifdef MAIN_FILE
	u8 Arena::poolA[ARENA_SIZE] ALIGNED(4096);
	u8 Arena::poolB[ARENA_SIZE] ALIGNED(4096) = HTTP_DEFAULT_ERROR_PAGES;
	usize Arena::sizeA = 0;
	usize Arena::sizeB = sizeof(HTTP_DEFAULT_ERROR_PAGES) + 1;
#endif
