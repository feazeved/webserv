#pragma once
#include "core.hpp"
#include "Arena.hpp"
#include "Bitmap.hpp"
#include "Connection.hpp"

class ConnectionPool {
public:
	static const usize blockSize = sizeof(Connection) * 64;
	static const usize blockCount = sizeof(Arena::pool.A) / blockSize;

public:
	Connection *const connections;
	Bitmap blockBitmap;
	Bitmap elementBitmap[blockCount];	// Metadata for each 64 Connection Block

	ConnectionPool() : connections((Connection*) Arena::pool.A) {
		// for (usize index = 0; index < blockCount * 64; index++)
		// 	new (connections + index) Connection();
	}

	~ConnectionPool() {
		clear();
		// for (usize index = 0; index < blockCount * 64; index++)
		// 	connections[index].~Connection();
	}

	Connection* get_ptr(usize linearIndex) {
		return connections + linearIndex;
	}

	void check_timeout(time_t curTime) {
		for (usize blockIndex = 0; blockIndex < blockCount; blockIndex++) {
			Bitmap &elementBlock = elementBitmap[blockIndex];
			if (elementBlock.bitmap == 0)
				continue;
			
			Connection *base = connections + blockIndex * 64;
			usize elementIndex = blockIndex;
			while ((elementIndex = elementBlock.find_first_set()) != SIZE_MAX) {
				if (base[elementIndex].check_timeout(curTime) == false)
					continue;
				// TODO: Update clock here, check_timeout might have lagged
				base[elementIndex].clear();
				elementBlock.bitclr(elementIndex);
			}
			blockBitmap.bitclr(blockIndex);
		}
	}

	// TODO: Review
	bool is_active(usize linearIndex) const {
		if (linearIndex >= blockCount * 64)
			return false;
		return elementBitmap[linearIndex / 64].bitread((u8)(linearIndex % 64));
	}

	void clear() {
		for (usize blockIndex = 0; blockIndex < blockCount; blockIndex++) {
			Bitmap &elementBlock = elementBitmap[blockIndex];
			if (elementBlock.bitmap == 0)
				continue;

			Connection *base = connections + blockIndex * 64;
			usize elementIndex;
			while ((elementIndex = elementBlock.find_first_set()) != SIZE_MAX) {
				base[elementIndex].clear();
				elementBlock.bitclr(elementIndex);
			}
			blockBitmap.bitclr(blockIndex);
		}
	}

	usize get_slot() {
		usize blockIndex = blockBitmap.find_first_clear();
		if (blockIndex >= blockCount)
			return SIZE_MAX;

		usize elementIndex = elementBitmap[blockIndex].find_first_clear();
		elementBitmap[blockIndex].bitset(elementIndex);
		if (elementBitmap[blockIndex].count() == 64)
			blockBitmap.bitset(blockIndex);

		return blockIndex * 64 + elementIndex;
	}

	usize acquire_slot(int clientFd, VirtualServer *server) {
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

STATIC_ASSERT(ConnectionPool::blockCount > 0);
STATIC_ASSERT(ConnectionPool::blockCount <= WORD_BITS);
