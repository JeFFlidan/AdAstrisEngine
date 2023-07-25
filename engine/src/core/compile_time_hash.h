#pragma once

#include <cstdint>

namespace ad_astris
{
	// Took from https://github.com/Themaister/Granite/tree/master
	constexpr uint64_t fnv_iterate(uint64_t hash, uint8_t c)
	{
		return (hash * 0x100000001b3ull) ^ c;
	}

	template<size_t index>
	constexpr uint64_t compile_time_fnv1_inner(uint64_t hash, const char* str)
	{
		return compile_time_fnv1_inner<index - 1>(fnv_iterate(hash, uint8_t(str[index])), str);
	}

	template<>
	constexpr uint64_t compile_time_fnv1_inner<size_t(-1)>(uint64_t hash, const char* str)
	{
		return hash;
	}

	template<size_t len>
	constexpr uint64_t compile_time_fnv1(const char (&str)[len])
	{
		return compile_time_fnv1_inner<len - 1>(0xcbf29ce484222325ull, str);
	}
}