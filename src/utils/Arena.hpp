#pragma once

#include "core.hpp"
#include <unistd.h>
#include "config.hpp"

/*
	Arena will have 64MB + Parsing Capacity. Ideally, anything that is supposed
	to be temporary is allocated within the ConnectionPool reserved space
	That way, the memory is essentially freed when the application starts running
*/

class Arena {
private:
	Arena();

public:
	static u8 data[ARENA_SIZE] ALIGNED(4096);
	static usize size;

	static void clear() {
		size = 0;
	}

	static void* alloc(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > sizeof(data) - size) {
			PRINT_LN(2, "Error: Out of memory");
			return NULL;
		}
		u8* ptr = data + size;
		size += bytes;
		return ptr;
	}

	static u32 alloc_index(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (bytes > sizeof(data) - size) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = size;
		size += bytes;
		return index;
	}
};

#ifdef MAIN_FILE
	u8 Arena::data[ARENA_SIZE] ALIGNED(4096);
	usize Arena::size = 0;
#endif
