#pragma once
#include "core.hpp"

class Bitmap {

public:
	usize bitmap;

	void bitset(u8 index) {
		bitmap |= (usize)1 << index;
	}

	void bitclr(u8 index) {
		bitmap &= ~((usize)1 << index);
	}

	void bitflip(u8 index) {
		bitmap ^= (usize)1 << index;
	}

	// Inclusive start, Exclusive end
	void bitwrite(u8 bitStart, u8 bitEnd, bool bit) {
		const usize startMask = SIZE_MAX << bitStart;
		const usize endMask = ((usize)1 << bitEnd) - 1;
		const usize mask = startMask & endMask;
		const usize bitMask = (usize)-bit;

		bitmap ^= (bitmap ^ bitMask) & mask;
	}

	bool bitread(u8 index) const {
		return (bitmap & ((usize)1 << index)) != 0;
	}

	usize find_first_clear() const {
		usize bit = (usize) FFS(~bitmap);
		return bit ? bit - 1 : WORD_BITS;
	}

	usize find_first_set() const {
		usize bit = (usize) FFS(bitmap);
		return bit ? bit - 1 : WORD_BITS;
	}

	usize count() const {
		return (usize)POPCOUNT(bitmap);
	}

	void clear() {
		bitmap = 0;
	}

	Bitmap() {
		clear();
	}
};
