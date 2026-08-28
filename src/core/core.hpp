#pragma once

#include <cstddef>
#include <stdint.h>
#include <climits>
#define restrict __restrict__

// Types
typedef char				i8;
typedef unsigned char		u8;
typedef int16_t				i16;
typedef uint16_t			u16;
typedef int32_t				i32;
typedef uint32_t			u32;
typedef int64_t				i64;
typedef uint64_t			u64;
typedef float				f32;
typedef double				f64;
typedef __int128			i128;
typedef unsigned __int128	u128;
typedef size_t				usize;
typedef ptrdiff_t			isize;
typedef uintptr_t			uptr;
typedef unsigned char		uchar;	// For completeness, to mirror platform's type
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

// Defines
#define ALIGN_SIZE	__alignof__(long double)
#define WORD_SIZE	sizeof(size_t)
#define WORD_BITS	(WORD_SIZE * CHAR_BIT)

#define PRINT_LN(fd, str)		((void)!write(fd, str "\n", sizeof(str)))
#define PERR_RETURN(value, str)	return (PRINT_LN(2, str), (value))
#include <cstdlib>	// TODO: Review these macros
#define PERR_EXIT(value, str)	std::exit((PRINT_LN(2, str), (value)))

#define STATIC_ASSERT_JOIN2(a, b) a##b
#define STATIC_ASSERT_JOIN(a, b) STATIC_ASSERT_JOIN2(a, b)
#define STATIC_ASSERT(expr) typedef char STATIC_ASSERT_JOIN(static_assert_failed_, __LINE__)[(expr) ? 1 : -1]

#ifdef DEBUG_MODE
	#define ON_DEBUG(x) (x)
	#define ASSERT(x, str) ((x) != 0 ? (void)0 : PRINT_LN(2, str))
#else
	#define ON_DEBUG(x) ((void)0)
	#define ASSERT(x, str) ((void)0)
#endif

#include "core_builtins.ipp"
#include "core_macros.ipp"
#include "core_info.ipp"
