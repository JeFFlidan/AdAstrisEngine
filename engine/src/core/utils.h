#pragma once

#include "tuple.h"
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
				for (int i = 0; i != vec.size(); ++i)
				{
					T x = vec[i];
					x = ((x >> 16) ^ x) * 0x45d9f3b;
					x = ((x >> 16) ^ x) * 0x45d9f3b;
					x = (x >> 16) ^ x;
					seed ^= x + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				}
				return seed;
			}

			// Counts the number of arguments in the parameter pack Args.
			template <typename... Args>
			static constexpr std::size_t count_args() 
			{
				return sizeof...(Args);
			}

			template<std::size_t N, class S>
			static auto custom_data_type_to_tuple(S&& s) 
			{
				return tuple_spec::CustomDataTypeToTuple<N>{}(std::forward<S>(s));
			}
	};
}