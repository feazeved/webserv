#pragma once

#include "core.hpp"
#include <unistd.h>
#include "config.hpp"
#include "status_codes.hpp"

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

// STATIC_ASSERT(sizeof())

struct Arena {
	union Pool {
		struct {
			u8 A[CONNECTION_POOL_SIZE] ALIGNED(4096);
			u8 B[CONFIG_POOL_SIZE] ALIGNED(4096);	
		};
		u8 base[CONNECTION_POOL_SIZE + CONFIG_POOL_SIZE] ALIGNED(4096);
	}	static pool;
	static usize sizeA, sizeB;
	static const usize poolBStaticSize = ALIGN_UP(sizeof(HTTP_ARENA_STATIC_STRINGS), 64);

	static void clear() {
		sizeA = 0;
		sizeB = poolBStaticSize;
	}

	static u8* mptr(usize fileOffset) {
		return pool.base + fileOffset;
	}

	static u32 alloc_a(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > sizeof(pool.A) - sizeA) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = sizeA;
		sizeA += bytes;
		return index;
	}

	static u32 alloc_b(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > sizeof(pool.B) - sizeB) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = sizeB + sizeof(pool.A);
		sizeB += bytes;
		return index;
	}
};

STATIC_ASSERT(sizeof(Arena::pool.A) + sizeof(Arena::pool.B) <= UINT32_MAX);
STATIC_ASSERT(sizeof(HTTP_ARENA_STATIC_STRINGS) <= UINT16_MAX);
// STATIC_ASSERT(arena.pool.BStaticSize <= sizeof(arena.pool.B));

#ifdef MAIN_FILE
	Arena::Pool Arena::pool = {
		{
			{ 0 },
			HTTP_ARENA_STATIC_STRINGS
		}
	};
	usize Arena::sizeA = 0;
	usize Arena::sizeB = Arena::poolBStaticSize;
#endif

// Arena::u_pool Arena::pool.A[ARENA_SIZE] ALIGNED(4096);
// Arena::u_pool Arena::pool.B[ARENA_SIZE] ALIGNED(4096) = HTTP_ARENA_STATIC_STRINGS;