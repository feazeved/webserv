#pragma once

#include "core.hpp"
#include <new>
#include <unistd.h>

// Class template
template <typename Type, usize blockSize, usize maxBlocks>
class BlockVector
{
public:
	Type stack[blockSize];
	Type* blocks[maxBlocks];
	usize blockCount;

	BlockVector() : blockCount(1)
	{
		blocks[0] = stack;

		for (usize i = 1; i < maxBlocks; i++)
			blocks[i] = NULL;
	}

	~BlockVector()
	{
		for (usize i = 1; i < blockCount; i++)
			delete[] blocks[i];
	}

	bool grow()
	{
		if (blockCount >= maxBlocks)
			return false;
		Type* block = new (std::nothrow) Type[blockSize];
		if (block == NULL)
			return false;
		blocks[blockCount] = block;
		blockCount++;
		return true;
	}

	bool shrink()
	{
		if (blockCount <= 1)
			return false;
		blockCount--;
		delete[] blocks[blockCount];
		blocks[blockCount] = NULL;
		return true;
	}

	// Added this so that I don't go out of bounds. it's not size() because this would imply its the number of initialized items
	usize	allocdSize()
	{
		return (blockSize * blockCount);
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
