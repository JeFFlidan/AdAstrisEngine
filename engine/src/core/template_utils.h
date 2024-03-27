#pragma once

#include <json/json.hpp>
#include <variant>
#include <functional>

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

	template<typename T, typename ...ARGS>
	void variant_from_json(const nlohmann::json& j, std::variant<ARGS...>& v)
	{
		v = j["value"].get<T>();
	}

	template<typename ...Types>
	class VariantSerializer
	{
		public:
			VariantSerializer()
			{
				size_t i = 0;
				(void(_funcs[i++] = variant_from_json<Types, Types...>), ...);
			}
		
			void serialize(nlohmann::json& j, const std::variant<Types...>& v)
			{
				std::visit([&](const auto& arg){ j["value"] = arg; }, v);
			}

			void deserialie(const nlohmann::json& j, std::variant<Types...>& v, int funcIndex)
			{
				_funcs[funcIndex](j, v);
			}

		private:
			std::function<void(const nlohmann::json&, std::variant<Types...>&)> _funcs[sizeof...(Types)];
	};
}