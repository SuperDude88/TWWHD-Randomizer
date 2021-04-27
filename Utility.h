#ifndef Utility_H
#define Utility_H

#include <cstdint>
#pragma once

namespace Utility

{

	inline uint64_t byteswap(const uint64_t& value)
	{
		return ((value & 0xFF00000000000000) >> 56) |
			((value & 0x00FF000000000000) >> 40) |
			((value & 0x0000FF0000000000) >> 24) |
			((value & 0x000000FF00000000) >> 8) |
			((value & 0x00000000FF000000) << 8) |
			((value & 0x0000000000FF0000) << 24) |
			((value & 0x000000000000FF00) << 40) |
			((value & 0x00000000000000FF) << 56);
	}

	inline uint32_t byteswap(const uint32_t& value)
	{
		return ((value & 0xFF000000) >> 24) |
			((value & 0x00FF0000) >> 8) |
			((value & 0x0000FF00) << 8) |
			((value & 0x000000FF) << 24);
	}

	inline uint16_t byteswap(const uint16_t& value)
	{
		return ((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8);
	}

	inline int64_t byteswap(const int64_t& value)
	{
		uint64_t result = byteswap(reinterpret_cast<const uint64_t&>(value));
		return *(reinterpret_cast<int64_t*>(&result));
	}

	inline int32_t byteswap(const int32_t& value)
	{
		uint32_t result = byteswap(reinterpret_cast<const uint32_t&>(value));
		return *(reinterpret_cast<int32_t*>(&result));
	}

	inline int16_t byteswap(const int16_t& value)
	{
		uint16_t result = byteswap(reinterpret_cast<const uint16_t&>(value));
		return *(reinterpret_cast<int16_t*>(&result));
	}

	inline float byteswap(const float& value)
	{
		uint32_t result = byteswap(reinterpret_cast<const uint32_t&>(value));
		return *(reinterpret_cast<float*>(&result));
	}

	inline double byteswap(const double& value)
	{
		uint64_t result = byteswap(reinterpret_cast<const uint64_t&>(value));
		return *(reinterpret_cast<double*>(&result));
	}

	template<typename T>
	void byteswap_inplace(T& value)
	{
		value = byteswap(value);
	}
}

#endif
