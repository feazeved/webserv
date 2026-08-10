#pragma once
#include "core.hpp"

template <bool condition, typename T, typename F>
struct Select {
    typedef T type;
};

template <typename T, typename F>
struct Select<false, T, F> {
    typedef F type;
};

template <usize maxValue>
struct SmallestUInt {
	typedef	typename Select<(maxValue <= UCHAR_MAX), u8,
			typename Select<(maxValue <= USHRT_MAX), u16,
			typename Select<(maxValue <= UINT_MAX), u32,
			u64>::type>::type>::type type;
};

template <usize bufferSize>
class Cursor {
private:
	typedef	typename Select<(bufferSize <= UCHAR_MAX), u8,
			typename Select<(bufferSize <= USHRT_MAX), u16,
			typename Select<(bufferSize <= UINT_MAX), u32, u64>
			::type>::type>::type Iterator;
public:
	Iterator index, size, start, end;

};

class Cursor16 {
public:
	u16 index, size, start, end;

};