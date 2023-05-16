#pragma once

#include <tuple>

namespace ad_astris
{
	template<typename ...ARGS>
	class Tuple
	{
		public:
			Tuple() = default;
			Tuple(ARGS&&... args) : _tuple(std::forward<ARGS>(args)...)
			{
				
			}

			template<typename T>
			T& get() const
			{
				return std::get<T>(_tuple);
			}

		private:
			std::tuple<ARGS...> _tuple;
	};
}