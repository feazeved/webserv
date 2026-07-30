#pragma once

#include "core.hpp"

template <usize bitCount>
class BitArray
{
private:
	typedef char bit_count_must_be_nonzero[(bitCount != 0) ? 1 : -1];
	typedef char word_bits_must_be_power_of_two[((WORD_BITS & (WORD_BITS - 1)) == 0) ? 1 : -1];
	typedef char usize_must_fit_ctz_type[(sizeof(usize) <= sizeof(unsigned long long)) ? 1 : -1];

public:
	static const usize metaSize = bitCount / WORD_BITS + (bitCount % WORD_BITS != 0);
	static const usize lengthMask = SIZE_MAX >> ((usize)(0 - bitCount) % WORD_BITS);
	usize bitmap[metaSize];

	void bitset() {
		MEMSET_INLINE(bitmap, 0xFF, sizeof(bitmap));
	}

	void bitset(usize index) {
		bitmap[index / WORD_BITS] |= ((usize)1 << (index % WORD_BITS));
	}

	void bitset(usize start, usize end) {
		const usize wordStart = start / WORD_BITS;
		const usize wordEnd = (end - 1) / WORD_BITS;
		const usize startMask = SIZE_MAX << (start % WORD_BITS);
		const usize endMask = SIZE_MAX >> ((usize)(0 - end) % WORD_BITS);

		if (wordStart == wordEnd) {
			bitmap[wordStart] |= startMask & endMask;
			return;
		}
		bitmap[wordStart] |= startMask;
		for (usize word = wordStart + 1; word < wordEnd; word++)
			bitmap[word] = SIZE_MAX;
		bitmap[wordEnd] |= endMask;
	}

	void bitclr() {
		MEMSET_INLINE(bitmap, 0, sizeof(bitmap));
		bitmap[metaSize - 1] = ~lengthMask;
	}

	void bitclr(usize index) {
		bitmap[index / WORD_BITS] &= ~((usize)1 << (index % WORD_BITS));
	}

	void bitclr(usize start, usize end)	{
		const usize wordStart = start / WORD_BITS;
		const usize wordEnd = (end - 1) / WORD_BITS;
		const usize startMask = SIZE_MAX << (start % WORD_BITS);
		const usize endMask = SIZE_MAX >> ((usize)(0 - end) % WORD_BITS);

		if (wordStart == wordEnd) {
			bitmap[wordStart] &= ~(startMask & endMask);
			return;
		}
		bitmap[wordStart] &= ~startMask;
		for (usize word = wordStart + 1; word < wordEnd; word++)
			bitmap[word] = 0;
		bitmap[wordEnd] &= ~endMask;
	}

	void bitflip() {
		for (usize i = 0; i < metaSize; i++)
			bitmap[i] = ~bitmap[i];
		bitmap[metaSize - 1] |= ~lengthMask;
	}

	void bitflip(usize index) {
		bitmap[index / WORD_BITS] ^= ((usize)1 << (index % WORD_BITS));
	}

	void bitflip(usize start, usize end) {
		const usize wordStart = start / WORD_BITS;
		const usize wordEnd = (end - 1) / WORD_BITS;
		const usize startMask = SIZE_MAX << (start % WORD_BITS);
		const usize endMask = SIZE_MAX >> ((usize)(0 - end) % WORD_BITS);
	
		if (wordStart == wordEnd) {
			bitmap[wordStart] ^= startMask & endMask;
			return;
		}
	
		bitmap[wordStart] ^= startMask;
		for (usize i = wordStart + 1; i < wordEnd; i++)
			bitmap[i] = ~bitmap[i];
		bitmap[wordEnd] ^= endMask;
	}

	void bitwrite(usize index, bool bit) {
		usize& word = bitmap[index / WORD_BITS];
		const usize mask = ((usize)1 << (index % WORD_BITS));
		word = (word & ~mask) | ((usize)-(usize)bit & mask);
	}

	bool bitread(usize index) const {
		return (bitmap[index / WORD_BITS] & ((usize)1 << (index % WORD_BITS))) != 0;
	}

	usize bitfind(usize start, usize end, bool bit) const {
		const usize invert = (usize)-(usize)!bit;
		const usize last = (end - 1) / WORD_BITS;
		const usize endMask = SIZE_MAX >> ((usize)(0 - end) % WORD_BITS);
		usize wordIndex = start / WORD_BITS;
		usize candidate = (bitmap[wordIndex] ^ invert) & (SIZE_MAX << (start % WORD_BITS));
	
		if (wordIndex == last)
			candidate &= endMask;
		while (true) {
			if (candidate != 0)
				return wordIndex * WORD_BITS + (usize)CTZ(candidate);
			if (wordIndex == last)
				return SIZE_MAX;
			candidate = bitmap[++wordIndex] ^ invert;
			if (wordIndex == last)
				candidate &= endMask;
		}
	}

	usize find_first_clear() const {
		for (usize word = 0; word < metaSize; word++) {
			const usize candidate = ~bitmap[word];
			if (candidate != 0)
				return word * WORD_BITS + (usize)CTZ(candidate);
		}
		return SIZE_MAX;
	}

	usize find_first_set() const {
		for (usize word = 0; word + 1 < metaSize; word++) {
			if (bitmap[word] != 0)
				return word * WORD_BITS + (usize)CTZ(bitmap[word]);
		}
		const usize candidate = bitmap[metaSize - 1] & lengthMask;
		return candidate != 0 ? (metaSize - 1) * WORD_BITS + (usize)CTZ(candidate) : SIZE_MAX;
	}

	usize count() const {
		usize result = 0;
		for (usize i = 0; i + 1 < metaSize; i++)
			result += POPCOUNT(bitmap[i]);
		result += POPCOUNT(bitmap[metaSize - 1] & lengthMask);
		return result;
	}

	usize size() const {
		return bitCount;
	}

	bool operator[](usize index) const {
		return bitread(index);
	}

	BitArray() {
		bitclr();
	}
};
