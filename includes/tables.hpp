#pragma once

// Should be included once in main.cpp

const unsigned char g_asciiLut[256] = {
	64, 20, 20, 20, 20, 20, 20, 20, 20, 18, 18, 18, 18, 18, 20, 20, // 0x00–0x0F
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, // 0x10–0x1F
	18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 0x20–0x2F
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 19, 19, 19, 19, 19, 19, // 0x30–0x3F: 0–9
	19, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, // 0x40–0x4F: @, A–O
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 19, 19, 19, 19, 17, // 0x50–0x5F: P–Z, [, \, ], ^, _
	19, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, // 0x60–0x6F: `, a–o
	16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 19, 19, 19, 19, 20, // 0x70–0x7F: p–z, {, |, }, ~, DEL
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0x80–0x8F
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0x90–0x9F
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0xA0–0xAF
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0xB0–0xBF
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0xC0–0xCF
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0xD0–0xDF
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0xE0–0xEF
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64 // 0xF0–0xFF
};

extern const unsigned char g_asciiLut[256];

// enum e_ascii2 {
// 	control = 1u,
// 	space = 1u << 1,
// 	symbols = 1u << 2,
// 	digits = 1u << 3,
// 	upper = 1u << 4,
// 	lower = 1u << 5,
// 	uscore = 1u << 6,
// 	hex_alpha = 1u << 7,
// 	alpha = upper | lower,
// 	alnum = alpha | digits,
// 	ident = uscore | alnum,
// 	hex = hex_alpha | digits
// };

// static const unsigned char g_asciiLut2[256] = {
// 	0, control, control, control, control, control, control, control, control,

// 	space, space, space, space, space,
// 	control, control, control, control, control, control, control, control, control, 
// 	control, control, control, control, control, control, control, control, control,
// 	space, symbols, symbols, symbols, symbols, symbols, symbols, symbols,
// 	symbols, symbols, symbols, symbols, symbols, symbols, symbols, symbols,

// 	digits, digits, digits, digits, digits, digits, digits, digits,	digits, digits,
// 	symbols, symbols, symbols, symbols, symbols, symbols, symbols, 

// 	upper|hex_alpha, upper|hex_alpha, upper|hex_alpha, upper|hex_alpha, 
// 	upper|hex_alpha, upper|hex_alpha,
// 	upper, upper, upper, upper, upper, upper, upper, upper, upper, upper, 
// 	upper, upper, upper, upper, upper, upper, upper, upper, upper, upper, 
// 	symbols, symbols, symbols, symbols, symbols | uscore, symbols,

// 	lower|hex_alpha, lower|hex_alpha, lower|hex_alpha, lower|hex_alpha, 
// 	lower|hex_alpha, lower|hex_alpha, lower, lower, lower, lower, lower,
// 	lower, lower, lower, lower, lower, lower, lower, lower, lower, lower, 
// 	lower, lower, lower, lower, lower,
// 	symbols, symbols, symbols, symbols, control
// };

