#pragma once

namespace ad_astris
{
	template<typename ...ARGS>
	struct ParameterPack
	{
		template <template <typename ...> typename T> using Apply = T<ARGS...>;
	};
}