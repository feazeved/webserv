#pragma once

#include "BitArray.hpp"
#include "Arena.hpp"
#include "core.hpp"

template <typename Type, usize blockSize, usize maxGrowth>
class BlockVector {
public:
	static const usize maxElements = blockSize * maxGrowth;

	u32 blocks[maxGrowth];
	BitArray<maxElements> metadata;
	usize numBlocks;
	usize numAllocatedBlocks;
	usize numElements;

	BlockVector() : numBlocks(0), numAllocatedBlocks(0), numElements(0) {
		for (usize index = 0; index < maxGrowth; index++)
			blocks[index] = UINT32_MAX;
		if (!grow())
			_exit(1);
	}

	Type* get_block(usize index) {
		return (Type*)(Arena::data + blocks[index]);
	}

	const Type* get_block(usize index) const {
		return (const Type*)(Arena::data + blocks[index]);
	}

	Type* get(usize index) {
		return get_block(index / blockSize) + index % blockSize;
	}

	const Type* get(usize index) const {
		return get_block(index / blockSize) + index % blockSize;
	}

	usize find_free_slot() {
		usize freeIndex = metadata.find_first_clear();
		if (freeIndex >= capacity()) {
			if (freeIndex >= maxElements || !grow())
				return SIZE_MAX;
		}
		return freeIndex;
	}

	usize acquire_slot() {
		usize index = find_free_slot();
		if (index == SIZE_MAX)
			return SIZE_MAX;
		metadata.bitset(index);
		numElements++;
		return index;
	}

	void init(usize index) {
		get(index)->init();
		metadata.bitset(index);
		numElements++;
	}

	void clear(usize index) {
		get(index)->clear();
		numElements--;
		metadata.bitclr(index);
	}

	bool grow() {
		if (numBlocks >= maxGrowth)
			return false;
		if (numBlocks == numAllocatedBlocks) {
			u32 offset = Arena::alloc_index(sizeof(Type) * blockSize);
			if (offset == UINT32_MAX)
				return false;
			blocks[numBlocks] = offset;
			numAllocatedBlocks++;
		}
		numBlocks++;
		return true;
	}

	bool shrink() {
		if (numBlocks <= 1)
			return false;
		numBlocks--;
		return true;
	}

	usize capacity() const {
		return blockSize * numBlocks;
	}

	usize size() const {
		return numElements;
	}

	usize index_of(const Type* element) const {
		uptr address = (uptr) element;
		for (usize block = 0; block < numBlocks; block++) {
			uptr begin = (uptr) get_block(block);
			uptr end = begin + sizeof(Type) * blockSize;
			if (address >= begin && address < end
				&& (address - begin) % sizeof(Type) == 0)
				return block * blockSize + (address - begin) / sizeof(Type);
		}
		return SIZE_MAX;
	}

	Type& operator[](usize index) {
		return *get(index);
	}

	const Type& operator[](usize index) const {
		return *get(index);
	}
};
