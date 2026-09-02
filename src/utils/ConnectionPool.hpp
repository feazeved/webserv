#pragma once
#include "core.hpp"
#include "Arena.hpp"
#include "Bitmap.hpp"
#include "Connection.hpp"

class ConnectionPool {
public:
	static const usize blockSize = sizeof(Connection) * 64;
	static const usize blockCount = 64;
	static const usize elementCount = 4096;

public:
	Connection connections[elementCount];
	Bitmap blockBitmap;
	Bitmap elementBitmap[blockCount];	// Metadata for each 64 Connection Block

	Connection* get_ptr(usize linearIndex) {
		return connections + linearIndex;
	}

	template <void (Connection::*Func)()>
	void for_each_active() {
		for (usize blockIndex = 0; blockIndex < blockCount; blockIndex++) {
			Bitmap active = elementBitmap[blockIndex];

			Connection *base = connections + blockIndex * 64;
			usize elementIndex;
			while ((elementIndex = active.find_first_set()) < WORD_BITS) {
				(base[elementIndex].*Func)();
				active.bitclr(elementIndex);
			}
		}
	}

	void clear() {
		for (usize blockIndex = 0; blockIndex < blockCount; blockIndex++) {
			Bitmap &elementBlock = elementBitmap[blockIndex];
			if (elementBlock.bitmap == 0)
				continue;

			Connection *base = connections + blockIndex * 64;
			usize elementIndex;
			while ((elementIndex = elementBlock.find_first_set()) < WORD_BITS) {
				base[elementIndex].end_connection();
				elementBlock.bitclr(elementIndex);
			}
			blockBitmap.bitclr(blockIndex);
		}
	}

	usize acquire_slot(int clientFd, VirtualServer *server) {
		usize blockIndex = blockBitmap.find_first_clear();
		if (blockIndex >= blockCount)
			return SIZE_MAX;

		usize elementIndex = elementBitmap[blockIndex].find_first_clear();
		elementBitmap[blockIndex].bitset(elementIndex);
		if (elementBitmap[blockIndex].bitmap == SIZE_MAX)
			blockBitmap.bitset(blockIndex);

		usize index = blockIndex * 64 + elementIndex;
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

STATIC_ASSERT(ConnectionPool::blockCount > 0);
STATIC_ASSERT(ConnectionPool::blockCount <= WORD_BITS);
