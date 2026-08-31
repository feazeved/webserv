#pragma once
#include "core.hpp"
#include "Arena.hpp"
#include "Bitmap.hpp"

template <typename Type>
struct MemoryPool {
	static const usize blockSize = sizeof(Type) * 64;
	static const usize blockCount = 64;

	Type elements[4096];
	Bitmap blockBitmap;
	Bitmap elementBitmap[blockCount];	// Metadata for each 64 Type Block

	Type* get_ptr(usize linearIndex) {
		return elements + linearIndex;
	}

	template <void (Type::*Func)()>
	void for_each_active() {
		for (usize blockIndex = 0; blockIndex < blockCount; blockIndex++) {
			Bitmap active = elementBitmap[blockIndex];
			if (active.bitmap == 0)
				continue;

			Type *base = elements + blockIndex * 64;
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

			Type *base = elements + blockIndex * 64;
			usize elementIndex;
			while ((elementIndex = elementBlock.find_first_set()) < WORD_BITS) {
				base[elementIndex].clear();
				elementBlock.bitclr(elementIndex);
			}
			blockBitmap.bitclr(blockIndex);
		}
	}

	usize acquire_slot() {
		usize blockIndex = blockBitmap.find_first_clear();
		if (blockIndex >= blockCount)
			return SIZE_MAX;

		usize elementIndex = elementBitmap[blockIndex].find_first_clear();
		elementBitmap[blockIndex].bitset(elementIndex);
		if (elementBitmap[blockIndex].bitmap == SIZE_MAX)
			blockBitmap.bitset(blockIndex);

		usize index = blockIndex * 64 + elementIndex;
		elements[index].init();
		return index;
	}

	void free_slot(usize linearIndex) {
		usize elementIndex = linearIndex % 64;
		usize blockIndex = linearIndex / 64;

		elements[linearIndex].clear();
		blockBitmap.bitclr(blockIndex);
		elementBitmap[blockIndex].bitclr(elementIndex);
	}

	Type& operator[](usize index) {
		return elements[index];
	}
};
