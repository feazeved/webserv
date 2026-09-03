#pragma once
#include "core.hpp"

// TODO: Create 32, 16 and 8 bit specializations
// TODO: bitmap array & 1000 for example through overloading

class Bitmap {
public:
	usize bitmap;

	sinl usize mask_start(usize bitStart) {
		return SIZE_MAX << bitStart % WORD_BITS;
	}

	sinl usize mask_end(usize bitEnd) {
		return SIZE_MAX >> ((usize)(0 - bitEnd) % WORD_BITS);
	}

	sinl usize mask_range(usize bitStart, usize bitEnd) {
		return mask_start(bitStart) & mask_end(bitEnd);
	}

	inl void bitset(usize index) {
		bitmap |= (usize)1 << index;
	}

	inl void bitclr(usize index) {
		bitmap &= ~((usize)1 << index);
	}

	inl void bitflip(usize index) {
		bitmap ^= (usize)1 << index;
	}

	// Inclusive start, Exclusive end
	inl void bitwrite(usize bitStart, usize bitEnd, bool bit) {
		const usize mask = mask_range(bitStart, bitEnd);
		const usize bitMask = (usize)-bit;

		bitmap ^= (bitmap ^ bitMask) & mask;
	}

	inl bool bitread(u8 index) const {
		return (bitmap & ((usize)1 << index)) != 0;
	}

	inl usize bitread(usize bitStart, usize bitEnd) const {
		return (bitmap & mask_range(bitStart, bitEnd)) >> bitStart;
	}

	inl static usize s_pop_first_set(usize &bitmap) {
		usize index = bitmap == 0 ? WORD_BITS : (usize)CTZ(bitmap);
		bitmap &= bitmap - 1;
		return index;
	}

	inl usize pop_first_set() {
		usize index = bitmap == 0 ? WORD_BITS : (usize)CTZ(bitmap);
		bitmap &= bitmap - 1;
		return index;
	}

	inl usize find_first_clear() {
		if (bitmap == SIZE_MAX)
			return WORD_BITS;
		return (usize)CTZ(~bitmap);
	}

	inl usize find_first_set() const {
		if (bitmap == 0)
			return WORD_BITS;
		return (usize)CTZ(bitmap);
	}

	// template <void (*Func)(usize)>
	// void for_each_active() const {
	// 	usize active = bitmap;

	// 	while (active != 0) {
	// 		usize index = (usize) FFS(active) - 1;
	// 		Func(index);
	// 		active &= active - 1;
	// 	}
	// }

/* ========== Accessors and Overloads ======================== */
	inl usize count() const {
		return (usize)POPCOUNT(bitmap);
	}

	inl void clear() {
		bitmap = 0;
	}

	inl void set() {
		bitmap = SIZE_MAX;
	}
	
	operator usize() {
		return bitmap;
	}

	Bitmap() {
		clear();
	}
};
