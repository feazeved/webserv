#pragma once
#include "core.hpp"

// === Builtin extensions =====================================
#define MEMFIND(dst, str, dstSize) \
({ \
	const u8 *mf_dst = (const u8 *)(dst); \
	const usize mf_dstSize = (usize)(dstSize); \
	const usize mf_strSize = sizeof(str) - 1; \
	usize mf_result = SIZE_MAX; \
    for (usize mf_i = 0; mf_i <= mf_dstSize - mf_strSize; mf_i++) { \
        if (MEMCMP(mf_dst + mf_i, (str), mf_strSize) == 0) { \
            mf_result = mf_i; \
            break; \
        } \
	}\
	mf_result; \
})

#define MEMCHR_INDEX(src, val, n)\
({\
	__auto_type _memchr_src = (const unsigned char *)(src);\
	__auto_type _memchr_result = (const unsigned char *) MEMCHR(_memchr_src, val, n);\
	_memchr_result ? (size_t)(_memchr_result - _memchr_src) : SIZE_MAX;\
})

// === Bit Helpers =========================================
#define BIT_READ(word, index)	(((word) >> (index)) & 1)
#define BIT_SET(word, index)	((word) | ((__auto_type(word))1 << (index)))
#define BIT_CLR(word, index)	((word) & ~((__auto_type(word))1 << (index)))

// === MINMAX Helpers ========================================
#define MIN(x, y)			((x) < (y) ? (x) : (y))
#define MAX(x, y)			((x) > (y) ? (x) : (y))
#define ABS(x)				((x) > 0 ? (x) : -(x))

#define CLAMP(x, low, high)	MAX(low, MIN(x, high))
#define ABSMAX(x, y)		MAX(ABS(x), ABS(y))
#define ABSMIN(x, y)		MIN(ABS(x), ABS(y))
#define ABSDIFF(x, y)		(MAX(x, y) - MIN(x, y))

#define MIN3(x, y, z)		MIN(x, MIN(y, z))
#define MAX3(x, y, z)		MAX(x, MAX(y, z))
#define MIN4(x, y, z, w)	MIN(MIN(x, y), MIN(z, w))
#define MAX4(x, y, z, w)	MAX(MAX(x, y), MAX(z, w))

#define ABSMIN3(x, y, z)	MIN3(ABS(x), ABS(y), ABS(z))
#define ABSMAX3(x, y, z)	MAX3(ABS(x), ABS(y), ABS(z))
#define ABSMIN4(x, y, z, w)	MIN4(ABS(x), ABS(y), ABS(z), ABS(w))
#define ABSMAX4(x, y, z, w)	MAX4(ABS(x), ABS(y), ABS(z), ABS(w))

// === Generic Helpers =====================================
#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]))
#define ARRAY_END(arr)		(&(arr)[ARRAY_SIZE(arr)])
#define SWAP(a, b) 			({__auto_type(a) _swap_tmp_ = (a); (a) = (b); (b) = _swap_tmp_; (void)0;})

#define STRINGIFY_(x)		#x
#define STRINGIFY(x)		STRINGIFY_(x)
#define ALIGN_UP(x, a)		(((x) + ((a) - 1)) & ~((a) - 1))	// TODO: rename this
#define ALIGN_DOWN(x, a)	((x) & ~((a) - 1))
#define IS_POW2(x)			(((x) & ((x) - 1)) == 0)			// UB for x==0
#define LOG2(x)				(63u - CLZ(x))	// TODO: maybe math helpers dont belong in this

// === ASCII Helpers =======================================
#define IS_ASCII(x) ((x) >= 0 && (x) < 128)
#define IS_DIGIT(x) ((x) >= '0' && (x) <= '9')
#define IS_UPPER(x) ((x) >= 'A' && (x) <= 'Z')
#define IS_LOWER(x) ((x) >= 'a' && (x) <= 'z')
#define IS_ALPHA(x) (IS_UPPER(x) || IS_LOWER(x))
#define IS_SPACE(x)	(g_asciiLut[((unsigned char)(x))] == ASCII_SPACE)
#define IS_HEX(x)	(g_asciiLut[((unsigned char)(x))] <= ASCII_HEX)
#define IS_ALNUM(x) (g_asciiLut[((unsigned char)(x))] <= ASCII_LETTERS)
#define IS_IDENT(x)	(g_asciiLut[((unsigned char)(x))] <= ASCII_IDENT)