#pragma once
#include "core.hpp"
#include "unistd.h"
#include "Bitmap.hpp"
#include "Connection.hpp"

namespace HTTP {

class ConnectionPool {
public:
	static const usize blockSize = sizeof(Connection) * 64;

public:
	static Connection ALIGNED(4096) connections[4096];		// 64 MB, gets lazily paged
	static Bitmap blockBitmap;
	static Bitmap elementBitmap[64];		// Metadata for each 64 Connection Block

	ConnectionPool() {
	}

	Connection* get_ptr(usize linearIndex) {
		return connections + linearIndex;
	}

	static void clear_block(usize blockIndex) {
		Bitmap &block = elementBitmap[blockIndex];
		usize elementIndex;
		Connection *base = connections + blockIndex * 64;

		while ((elementIndex = block.find_first_set()) != 0) {
			base[elementIndex].clear();
		}
	}

	void clear() {
		usize blockIndex;

		while ((blockIndex = blockBitmap.find_first_set()) != 0) {
			clear_block(blockIndex);
		}
	}

	usize acquire_slot() {
		usize blockIndex = blockBitmap.find_first_clear();
		if (blockIndex >= 64)
			return SIZE_MAX;

		usize elementIndex = elementBitmap[blockIndex].find_first_clear();
		elementBitmap[blockIndex].bitset(elementIndex);
		if (elementBitmap[blockIndex].count() == 64)
			blockBitmap.bitset(blockIndex);

		return blockIndex * 64 + elementIndex;
	}

	void free_slot(usize linearIndex) {
		usize elementIndex = linearIndex % 64;
		usize blockIndex = linearIndex / 64;

		blockBitmap.bitclr(blockIndex);
		elementBitmap[blockIndex].bitclr(elementIndex);
	}

	Connection& operator[](usize index) {
		return connections[index];
	}
};
}