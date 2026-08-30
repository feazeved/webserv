#pragma once
#include "core.hpp"
#include "Bitmap.hpp"

// Key will have 12 bit address range, 12 bit length, 8 bit modulo range
// So 8 bits index

#define MODBITS 12
#define ADDRBITS 12
#define LENBITS 8
#define ALIGN 16

struct Region {
	u8 data[16 * 4096];
	Bitmap big;
	Bitmap small[64];
};

struct Allocator {
	Region region[256];
};
