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
#define ALIGN_SIZE	alignof(std::max_align_t)
#define WORD_SIZE	sizeof(size_t)
#define WORD_BITS	(WORD_SIZE * CHAR_BIT)

enum e_ascii {
	ASCII_DIGITS      = 9,   // value <= digits
	ASCII_HEX         = 15,  // value <= hex
	ASCII_LETTERS     = 35,  // A-Z / a-z map to 10-35
	ASCII_IDENT       = 36,  // _
	ASCII_RFC_SYMBOLS = 37,  // RFC 3986 path symbols
	ASCII_SYMBOLS     = 38,  // other symbols
	ASCII_SPACE       = 39,
	ASCII_CONTROL     = 40,
	ASCII_INVALID     = 255
};

// Tables
#ifdef MAIN_FILE
	const u8 g_asciiLut[256] = {
		255, 40, 40, 40, 40, 40, 40, 40, 40, 39, 39, 39, 39, 39, 40, 40, // 0x00-0x0F
		40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, // 0x10-0x1F
		//     SP   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
		/*20*/ 39, 37, 38, 38, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
		//      0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
		/*30*/  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 37, 37, 38, 37, 38, 38,
		//      @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
		/*40*/ 37, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
		//      P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
		/*50*/ 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 38, 38, 38, 36,
		//      `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
		/*60*/ 38, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
		//      p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~  DEL
		/*70*/ 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 38, 38, 37, 40,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
	};
#else
	extern const u8 g_asciiLut[256];
#endif

// Backup in case chatgpt fucked my table

// const u8 g_asciiLut[256] = {
// 	127, 39, 39, 39, 39, 39, 39, 39, 39, 37, 37, 37, 37, 37, 39, 39, // 0x00–0x0F
// 	39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, // 0x10–0x1F
// 	37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, // 0x20–0x2F
// 	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 38, 38, 38, 38, 38, 38, // 0x30–0x3F: 0–9
// 	38, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 0x40–0x4F: @, A–O
// 	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 38, 38, 38, 36, // 0x50–0x5F: P–Z, [, \, ], ^, _
// 	38, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, // 0x60–0x6F: `, a–o
// 	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 38, 38, 38, 39, // 0x70–0x7F: p–z, {, |, }, ~, DEL

// 	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, // 0x80–0x8F
// 	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, // 0x90–0x9F
// 	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, // 0xA0–0xAF
// 	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, // 0xB0–0xBF
// 	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, // 0xC0–0xCF
// 	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, // 0xD0–0xDF
// 	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, // 0xE0–0xEF
// 	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255  // 0xF0–0xFF
// };

#include "core_builtins.ipp"
#include "core_macros.ipp"
#include "core_info.ipp"