#pragma once

#include <json/json.hpp>
#include <variant>

namespace ad_astris
{
	template<typename ...TypesPack>
	struct ParameterPack
	{
		template <template <typename ...> typename T> using Apply = T<TypesPack...>;

		template<typename T>
		constexpr static bool has_type()
		{
			return (std::is_same_v<T, TypesPack> || ...);
		}
	};
	
	constexpr const char* VARIANT_VALUE_KEY = "value";
	constexpr const char* VARIANT_VALUE_INDEX_KEY = "__variant_index";

	template<typename T, typename ...ARGS>
	void variant_from_json(const nlohmann::json& j, std::variant<ARGS...>& v, int index)
	{
		if (index == j[VARIANT_VALUE_INDEX_KEY].get<int>())
			v = j[VARIANT_VALUE_KEY].get<T>();
	}
}

namespace nlohmann
{
	template<typename ...Types>
	struct adl_serializer<std::variant<Types...>>
	{
		static void to_json(json& j, const std::variant<Types...>& v)
		{
			std::visit([&](const auto& arg)
			{
				j[ad_astris::VARIANT_VALUE_KEY] = arg;
				j[ad_astris::VARIANT_VALUE_INDEX_KEY] = v.index();
			}, v);
		}

		static void from_json(const json& j, std::variant<Types...>& v)
		{
			int i = 0;
			(ad_astris::variant_from_json<Types>(j, v, i++), ...);
		}
	};
}