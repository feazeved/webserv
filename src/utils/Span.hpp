#pragma once
#include "core.hpp"


struct Span {
	char* ptr;
	usize length;
};

struct Span32 {
	u32 index;
	u32 length;
};

struct Span16 {
	u16 index;
	u16 length;
};