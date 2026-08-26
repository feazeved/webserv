#pragma once
#include "core.hpp"

/*
	Remember the part about allocations being aligned, you can use the bits to index
	a bigger range than possible by considering blocks instead of bytes.
	A u16 can index 4MB if the allocations are cache aligned, and 1MB if they're 16

	And we can push this even further!
	A u8 can index 16KB of memory (cache aligned), but suppose we bin it given its size
	If its upper bits can be used to index the memory region as well, then we extend it

	So for example

	Memory region for:
	
	128+ size | 64+ size | 32+ size | 16+ size | 16- size 

	This would make a cache aligned u8 index potentially 128kb!
		The catch however is that each region holds static memory, so if 128 sizes are less common
	that's memory wasted

	But maybe a better solution would be

	8- size | 9 | 10 | 11 | 12 | ... | 64

	The best solution would probably be the 4 lower bits
	The higher you go the more memory you assign to more specialized ranges
	
	The problem might be with aligned sizes
	0000 will probably be crowded

	Cache Align: 	64x
	4 Low Bit:		16x
	___________________
					1024x Expansion

	Literally 1kb expansion, u8 indexing 256kb

*/

struct Span {
	char* ptr;
	usize length;
};

struct Span32 {
	u32 index;
	u32 length;
};

struct Span16 {
	u16 index;
	u16 length;

	Span extract(char* ptr) {
		Span result;
		result.ptr = ptr + index;
		result.length = length;
		return result;
	}
};