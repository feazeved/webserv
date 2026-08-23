#pragma once
#include <unistd.h>

#include "core.hpp"
#include "Arena.hpp"
#include "Bitmap.hpp"
#include "Connection.hpp"

namespace HTTP {

class VirtualServer;


class ConnectionPool {
public:
	static const usize blockSize = sizeof(Connection) * 64;
	static const usize blockCount = 64;

public:
	Connection *const connections;
	Bitmap blockBitmap;
	Bitmap elementBitmap[64];		// Metadata for each 64 Connection Block

	ConnectionPool() : connections((Connection*) Arena::poolA) {
	}

	~ConnectionPool() {
		clear();
	}

	Connection* get_ptr(usize linearIndex) {
		return connections + linearIndex;
	}

	void clear_block(usize blockIndex) {
		Bitmap &block = elementBitmap[blockIndex];
		usize elementIndex;
		Connection *base = connections + blockIndex * 64;

		while ((elementIndex = block.find_first_set()) != SIZE_MAX) {
			base[elementIndex].clear();
			block.bitclr(elementIndex);
		}
	}

	void clear() {
		usize blockIndex;

		while ((blockIndex = blockBitmap.find_first_set()) != SIZE_MAX) {
			clear_block(blockIndex);
			blockBitmap.bitclr(blockIndex);
		}
	}

	usize get_slot() {
		usize blockIndex = blockBitmap.find_first_clear();
		if (blockIndex >= 64)
			return SIZE_MAX;

		usize elementIndex = elementBitmap[blockIndex].find_first_clear();
		elementBitmap[blockIndex].bitset(elementIndex);
		if (elementBitmap[blockIndex].count() == 64)
			blockBitmap.bitset(blockIndex);

		return blockIndex * 64 + elementIndex;
	}

	usize acquire_slot(i32 clientFd, VirtualServer *server) {
		const usize index = get_slot();
		if (index != SIZE_MAX)
			connections[index].init(clientFd, server);
		return index;
	}

	void free_slot(usize linearIndex) {
		usize elementIndex = linearIndex % 64;
		usize blockIndex = linearIndex / 64;

		connections[linearIndex].clear();
		blockBitmap.bitclr(blockIndex);
		elementBitmap[blockIndex].bitclr(elementIndex);
	}

	Connection& operator[](usize index) {
		return connections[index];
	}
};
}
