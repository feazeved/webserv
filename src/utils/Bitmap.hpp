#pragma once
#include "core.hpp"

class Bitmap {
public:
	usize bitmap;

	ALWAYS_INLINE
	static usize mask_start(usize bitStart) {
		return SIZE_MAX << bitStart % WORD_BITS;
	}

	ALWAYS_INLINE
	static usize mask_end(usize bitEnd) {
		return SIZE_MAX >> ((usize)(0 - bitEnd) % WORD_BITS);
	}

	ALWAYS_INLINE
	static usize mask_range(usize bitStart, usize bitEnd) {
		return mask_start(bitStart) & mask_end(bitEnd);
	}

	ALWAYS_INLINE
	void bitset(u8 index) {
		bitmap |= (usize)1 << index;
	}

	ALWAYS_INLINE
	void bitclr(u8 index) {
		bitmap &= ~((usize)1 << index);
	}

	ALWAYS_INLINE
	void bitflip(u8 index) {
		bitmap ^= (usize)1 << index;
	}

	ALWAYS_INLINE	// Inclusive start, Exclusive end
	void bitwrite(u8 bitStart, u8 bitEnd, bool bit) {
		const usize mask = mask_range(bitStart, bitEnd);
		const usize bitMask = (usize)-bit;

		bitmap ^= (bitmap ^ bitMask) & mask;
	}

	ALWAYS_INLINE
	bool bitread(u8 index) const {
		return (bitmap & ((usize)1 << index)) != 0;
	}

	ALWAYS_INLINE
	usize find_first_clear() const {
		usize bit = (usize) FFS(~bitmap);
		return bit ? bit - 1 : WORD_BITS;
	}

	ALWAYS_INLINE
	usize find_first_set() const {
		usize bit = (usize) FFS(bitmap);
		return bit ? bit - 1 : WORD_BITS;
	}

	ALWAYS_INLINE
	usize count() const {
		return (usize)POPCOUNT(bitmap);
	}

	ALWAYS_INLINE
	void clear() {
		bitmap = 0;
	}

	ALWAYS_INLINE
	void set() {
		bitmap = SIZE_MAX;
	}

	Bitmap() {
		clear();
	}
};
