#include <cstddef>
#include <cstdint>
#include <climits>
#define restrict __restrict__

#define ALIGN_SIZE	alignof(std::max_align_t)
#define WORD_SIZE	sizeof(size_t)
#define WORD_BITS	(WORD_SIZE * CHAR_BIT)

// Types
typedef int8_t			i8;
typedef uint8_t			u8;
typedef int16_t			i16;
typedef uint16_t		u16;
typedef int32_t			i32;
typedef uint32_t		u32;
typedef int64_t			i64;
typedef uint64_t		u64;
typedef float			f32;
typedef double			f64;	// Should I include 128?
typedef size_t			usize;
typedef ptrdiff_t		isize;
typedef unsigned char	uchar;	// For completeness, to mirror platform's type
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned long	ulong;

// Builtins
#define ALWAYS_INLINE	static inline __attribute__((always_inline))
#define NOINLINE		__attribute__((noinline))
#define PURE			__attribute__((pure))			// Function produces no observable side effects (may read from memory)
#define KPURE			__attribute__((const))			// Function depends only on its arguments (doesn't read from memory)
#define PACKED			__attribute__((packed))			// Struct has no padding
#define ALIGNED(n)		__attribute__((aligned(n)))
#define COLD			__attribute__(cold)
#define HOT				__attribute__(hot)
#define FLATTEN			__attribute__((flatten))		// Function calls inside this function are aggressively inlined

#define UNREACHABLE()	__builtin_unreachable()
#define LIKELY(x)		__builtin_expect(!!(x), 1)
#define UNLIKELY(x)		__builtin_expect(!!(x), 0)
#if defined(__clang__)
	#define ASSUME(x)	__builtin_assume(x)
#elif defined(__GNUC__)
	#define ASSUME(x) ((x) ? (void)0 : __builtin_unreachable())
#endif

#define MEMCPY_BUILTIN(dst, src, n)		__builtin_memcpy(dst, src, n)
#define MEMMOVE_BUILTIN(dst, src, n)	__builtin_memmove(dst, src, n)
#define MEMSET_BUILTIN(dst, val, n)		__builtin_memset(dst, val, n)
#define MEMCHR_BUILTIN(src, val, n)		__builtin_memchr(src, val, n)
#define MEMCMP_BUILTIN(s1, s2, n)		__builtin_memcmp(s1, s2, n)

#if defined(__clang__) && __has_builtin(__builtin_memcpy_inline)
	#define MEMCPY_INLINE(dst, src, n)	__builtin_memcpy_inline(dst, src, n)
#else
	#define MEMCPY_INLINE(dst, src, n)	__builtin_memcpy(dst, src, n)
#endif

#if defined(__clang__) && __has_builtin(__builtin_memset_inline)
	#define MEMSET_INLINE(dst, val, n)	__builtin_memset_inline(dst, val, n)
#else
	#define MEMSET_INLINE(dst, val, n)	__builtin_memset(dst, val, n)
#endif

#define CLZ(x)			__builtin_clzll(x)
#define CTZ(x)			__builtin_ctzll(x)
#define POPCOUNT(x)		__builtin_popcountll(x)
#define FFS(x)			__builtin_ffsll(x)
#define PARITY(x)		__builtin_parityll(x)
#define BSWAP16(x)		__builtin_bswap16(x)
#define BSWAP32(x)		__builtin_bswap32(x)
#define BSWAP64(x)		__builtin_bswap64(x)
#define BITREVERSE(x)	__builtin_bitreverse64(x)	// TODO: CLANG SPECIFIC

// === Bit Helpers =========================================
#define BIT_READ(word, index)	(((word) >> (index)) & 1)
#define BIT_SET(word, index)	((word) | ((typeof(word))1 << (index)))
#define BIT_CLR(word, index)	((word) & ~((typeof(word))1 << (index)))

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

// === Math Helpers ========================================
#define LOG2(x)				(63u - CLZ(x))	// TODO: maybe math helpers dont belong in this

// === Generic Helpers =====================================
#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]))
#define ARRAY_END(arr)		(&(arr)[ARRAY_SIZE(arr)])
#define SWAP(a, b) 			({auto _swap_tmp_ = (a); (a) = (b); (b) = ___tmp; (void)0;})

#define STRINGIFY_(x)		#x
#define STRINGIFY(x)		STRINGIFY_(x)
#define ALIGN_UP(x, a)		(((x) + ((a) - 1)) & ~((a) - 1))	// TODO: rename this
#define ALIGN_DOWN(x, a)	((x) & ~((a) - 1))
#define IS_POW2(x)			(((x) & ((x) - 1)) == 0)			// UB for x==0