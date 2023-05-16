#pragma once

#include <vector>

namespace ad_astris
{
	class CoreUtils
	{
		public:
			template<typename T>
			static size_t hash_numeric_vector(std::vector<T>& vec)
			{
				size_t seed = vec.size();
				for (auto& x : vec)
				{
					x = ((x >> 16) ^ x) * 0x45d9f3b;
					x = ((x >> 16) ^ x) * 0x45d9f3b;
					x = (x >> 16) ^ x;
					seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				}
				return seed;
			}
	};
}