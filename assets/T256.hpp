#pragma once
#include "core.hpp"

typedef uint64_t __attribute__((vector_size(32))) u256;

// static const u256 s_ones = {0x0101010101010101UL, 0x0101010101010101UL, 0x0101010101010101UL, 0x0101010101010101UL};

// #define ONES 0x0101010101010101UL

// class T256 {
// public:
// 	static const u128 s_ones64 = (u128) 0x0101010101010101UL;
// 	static const u128 s_ones = (s_ones64 << 64) | (s_ones64);

// 	union {
// 		u8 data8[32];
// 		u128 data128[2];
// 	};

// 	T256& operator|=(u8 bytes) {
// 		const u128 mask = s_ones * bytes;
// 		data128[0] |= mask;
// 		data128[1] |= mask;
// 		return *this;
// 	}

// 	T256 operator|(u8 byte) {
// 		const u128 mask = s_ones * byte;
// 		T256 rvalue(data128);
// 		rvalue.data128[0] |= mask;
// 		rvalue.data128[1] |= mask;
// 		return rvalue;
// 	}

// 	bool operator==(T256 &other) {
// 		return data128[0] == other.data128[0]
// 			&& data128[1] == other.data128[1];
// 	}
// };

// The better call is to create a set function with the ones

class T256 {
public:
	static const u128 s_ones64 = (u128) 0x0101010101010101UL;
	static const u128 s_ones = (s_ones64 << 64) | (s_ones64);

public:
	union {
		u8 data8[32];
		u128 data128[2];
	};

	T256()
	{
		data128[0] = 0;
		data128[1] = 0;
	}

	explicit T256(u8 byte)
	{
		const u128 mask = byte * s_ones;
		data128[0] = mask;
		data128[1] = mask;
	}

	T256(u128 word0, u128 word1)
	{
		data128[0] = word0;
		data128[1] = word1;
	}

	explicit T256(const u128 (&words)[2])
	{
		data128[0] = words[0];
		data128[1] = words[1];
	}

	T256(const T256& other)
	{
		data128[0] = other.data128[0];
		data128[1] = other.data128[1];
	}

	T256& operator=(const T256& other)
	{
		data128[0] = other.data128[0];
		data128[1] = other.data128[1];
		return *this;
	}

	T256& operator=(u128 number)
	{
		data128[0] = number;
		data128[1] = number;
		return *this;
	}

	T256& operator=(u8 byte)
	{
		const u128 mask = byte * s_ones;
		data128[0] = mask;
		data128[1] = mask;
		return *this;
	}

	u8& operator[](std::size_t index)
	{
		return data8[index];
	}

	const u8& operator[](std::size_t index) const
	{
		return data8[index];
	}

	T256& operator|=(u8 byte)
	{
		const u128 mask = byte * s_ones;
		data128[0] |= mask;
		data128[1] |= mask;
		return *this;
	}

	T256 operator|(u8 byte) const
	{
		const u128 mask = byte * s_ones;
		return T256(data128[0] | mask, data128[1] | mask);
	}

	T256& operator|=(const T256& other)
	{
		data128[0] |= other.data128[0];
		data128[1] |= other.data128[1];
		return *this;
	}

	T256 operator|(const T256& other) const
	{
		return T256(data128[0] | other.data128[0],
					data128[1] | other.data128[1]);
	}

	T256& operator&=(const T256& other)
	{
		data128[0] &= other.data128[0];
		data128[1] &= other.data128[1];
		return *this;
	}

	T256 operator&(const T256& other) const
	{
		return T256(data128[0] & other.data128[0],
					data128[1] & other.data128[1]);
	}

	T256& operator^=(const T256& other)
	{
		data128[0] ^= other.data128[0];
		data128[1] ^= other.data128[1];
		return *this;
	}

	T256 operator^(const T256& other) const
	{
		return T256(data128[0] ^ other.data128[0],
					data128[1] ^ other.data128[1]);
	}

	T256 operator~() const
	{
		return T256(~data128[0], ~data128[1]);
	}

	bool operator==(const T256& other) const
	{
		return data128[0] == other.data128[0] &&
				data128[1] == other.data128[1];
	}

	bool operator==(u128 number) const
	{
		return (number == data128[0]) && (number == data128[1]);
	}

	bool operator!=(const T256& other) const
	{
		return !(*this == other);
	}

	friend T256 operator|(u8 byte, const T256& value)
	{
		return value | byte;
	}
};

// union T256 {
// public:
// 	u256 data;

// 	T256(u256 value) : data(value) {
// 	}

// 	T256 operator|(u8 byte) {
// 		const u256 mask = s_ones * byte;
// 		T256 rvalue(data|mask);
// 		return rvalue;
// 	}

// 	bool operator==(T256 &other) {
// 		const u256 eq = this->data == other.data;
// 		return eq[0] && eq[1] && eq[2] && eq[3];
// 	}
// };