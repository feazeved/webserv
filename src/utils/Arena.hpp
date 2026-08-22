#pragma once

#include "core.hpp"
#include <unistd.h>
#include "config.hpp"

class Arena {
private:
	Arena();

public:
	static u8 data[ARENA_SIZE];
	static usize size;

	static void* alloc(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (size + bytes >= sizeof(data)) {
			PRINT_LN(2, "Error: Out of memory");
			size = 0;
			return NULL;
		}
		u8* ptr = data + size;
		size += bytes;
		return ptr;
	}

	static u32 alloc_index(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (size + bytes >= sizeof(data)) {
			PRINT_LN(2, "Error: Out of memory");
			size = 0;
			return UINT32_MAX;
		}
		u32 index = size;
		size += bytes;
		return index;
	}
};

#ifdef MAIN_FILE
	usize Arena::size = 0;
#endif
