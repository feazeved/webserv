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

	ALWAYS_INLINE void bitset(usize index)	{ bitmap |= (usize)1 << index; }
	ALWAYS_INLINE void bitclr(usize index)	{ bitmap &= ~((usize)1 << index);}
	ALWAYS_INLINE void bitflip(usize index)	{ bitmap ^= (usize)1 << index; }

	ALWAYS_INLINE	// Inclusive start, Exclusive end
	void bitwrite(usize bitStart, usize bitEnd, bool bit) {
		const usize mask = mask_range(bitStart, bitEnd);
		const usize bitMask = (usize)-bit;

		bitmap ^= (bitmap ^ bitMask) & mask;
	}

	ALWAYS_INLINE
	static usize pop_first_set(usize &bitmap) {
		usize index = bitmap == 0 ? WORD_BITS : (usize)CTZ(bitmap);
		bitmap &= bitmap - 1;
		return index;
	}

	ALWAYS_INLINE
	bool bitread(u8 index) const {
		return (bitmap & ((usize)1 << index)) != 0;
	}

	ALWAYS_INLINE
	usize bitread(usize bitStart, usize bitEnd) const {
		return (bitmap & mask_range(bitStart, bitEnd)) >> bitStart;
	}

	ALWAYS_INLINE
	usize find_first_clear() {
		if (bitmap == SIZE_MAX)
			return WORD_BITS;
		return (usize)CTZ(~bitmap);
	}

	ALWAYS_INLINE
	usize find_first_set() const {
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
