#include "core.hpp"
#include <unistd.h>

#define BUFFER_CAPACITY 1024
#define MAX_CAPACITY 1024*1024ul

// Class assigns a stack buffer and uses heap if growth is needed.
// Auto handles reads
// template <usize stackCapacity>
class Buffer
{
public:
	u8 stack[BUFFER_CAPACITY - 16];
	u8 *data;
	u32 size, capacity;

// Methods
void compact(u32 index)
{
	u32 newSize = size - index;

	if (stack != data && newSize < (sizeof(stack) / 2))
	{
		MEMCPY_BUILTIN(stack, data + index, newSize);
		delete[] data;
		data = stack;
	}
	else
		MEMMOVE_BUILTIN(data, data + index, newSize);
	size = newSize;
}

bool reserve(usize required)
{
	if (required <= capacity)
		return true;
	if (required > MAX_CAPACITY)
		return false;

	usize newCapacity = ALIGN_UP(required + capacity, 4096);
	if (newCapacity > MAX_CAPACITY)
		return false;
	u8* newData = new u8[newCapacity];
	if (newData == NULL)
		return false;

	MEMCPY_BUILTIN(newData, data, size);
	if (data != stack)
		delete[] data;

	data = newData;
	capacity = newCapacity;
	return true;
}

bool append(const void* src, usize count)
{
	if (!reserve(size + count))
		return false;

	MEMCPY_BUILTIN(data + size, src, count);
	size += count;
	return true;
}

isize read(int fd, usize bytes)
{
	if (!reserve(bytes + size))
		return (-1);
	isize rvalue = ::read(fd, data + size, bytes);	// Interesting stuff
	if (rvalue > 0)
		size += (usize) rvalue;
	return rvalue;
}

// Constructors and Overloads
Buffer() :
	data(stack),
	size(0),
	capacity(sizeof(stack))
{
}

~Buffer()
{
	if (data != stack)
		delete[] data;
}

u8& operator[](u32 index)
{
	return data[index];
}

const u8& operator[](u32 index) const
{
	return data[index];
}
};
