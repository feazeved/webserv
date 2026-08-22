#pragma once
#include "core.hpp"
#include "unistd.h"
#include "Bitmap.hpp"

// Placeholder
struct Connection {
	u8 memory[16 * 1024];

	void init();
	void clear();
};

class ConnectionPool {
public:
	static const usize blockSize = sizeof(Connection) * 64;

public:
	Bitmap		blockBitmap;
	Bitmap		elementBitmap[64];		// Metadata for each 64 Connection Block
	Connection	connections[4096];		// 64 MB, gets lazily paged

	ConnectionPool() : elementBitmap() {
	}

	Connection* acquire_slot() {
		usize blockIndex = blockBitmap.find_first_clear();
		if (blockIndex >= 64)
			return NULL;

		usize elementIndex = elementBitmap[blockIndex].find_first_clear();
		elementBitmap[blockIndex].bitset(elementIndex);
		usize count = elementBitmap[blockIndex].count();
		if (count == 64)
			blockBitmap.bitset(blockIndex);

		usize linearIndex = blockIndex * 64 + elementIndex;
		return connections + linearIndex;
	}

	void free_slot(Connection* ptr) {
		const isize delta = (u8*) ptr - (u8*) connections;
		ASSERT(delta >= 0 && (usize)delta < sizeof(connections) 
			&& ((usize) delta % sizeof(Connection) == 0), "Error: Gave wrong pointer to Pool");

		usize elementIndex = ((usize) delta % blockSize) / sizeof(Connection);
		usize blockIndex = (usize) delta / blockSize;

		blockBitmap.bitclr(blockIndex);
		elementBitmap[blockIndex].bitclr(elementIndex);
	}
};

