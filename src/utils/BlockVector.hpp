#pragma once

#include "BitArray.hpp"
#include "core.hpp"
#include <new>
#include <unistd.h>

// Class template

template <typename Type, usize blockSize, usize maxGrowth>
class BlockVector {
public:
	static const usize maxElements = blockSize * maxGrowth;

	Type stack[blockSize];
	Type* blocks[maxGrowth];
	BitArray<maxElements> metadata;
	usize numBlocks;
	usize numElements;

	BlockVector() : numBlocks(1), numElements(0)
	{
		blocks[0] = stack;

		for (usize i = 1; i < maxGrowth; i++)
			blocks[i] = NULL;
	}

	~BlockVector()
	{
		for (usize i = 1; i < numBlocks; i++)
			delete[] blocks[i];
	}

	// Caller must call init still
	usize find_free_slot() {
		usize freeIndex = metadata.find_first_clear();

		if (freeIndex > capacity()) {
			if (freeIndex > maxElements)
				return SIZE_MAX;
			if (grow() == false)
				return SIZE_MAX;
		}
		return freeIndex;
	}

	void init(usize index)
	{
		blocks[index / blockSize][index % blockSize].init();
		numElements++;
		metadata.bitset(index);
	}

	void clear(usize index)
	{
		blocks[index / blockSize][index % blockSize].clear();
		numElements--;
		metadata.bitclr(index);
	}

	bool grow()
	{
		if (numBlocks >= maxGrowth)
			return false;
		Type* block = new (std::nothrow) Type[blockSize];
		if (block == NULL)
			return false;
		blocks[numBlocks] = block;
		numBlocks++;
		return true;
	}

	bool shrink()
	{
		if (numBlocks <= 1)
			return false;
		numBlocks--;
		delete[] blocks[numBlocks];
		blocks[numBlocks] = NULL;
		return true;
	}

	usize capacity() {
		return blockSize * numBlocks;
	}

	usize size() {
		return numElements;
	}

	Type& operator[](usize index)
	{
		return blocks[index / blockSize][index % blockSize];
	}

	const Type& operator[](usize index) const
	{
		return blocks[index / blockSize][index % blockSize];
	}
};
