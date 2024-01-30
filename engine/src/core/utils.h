#pragma once

#include "tuple.h"
#include <vector>
#include <ctime>
#include <typeinfo>

#if defined(_WIN32)
#include <windows.h>
#endif

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

			static void convert_string(const std::wstring& from, std::string& to)
			{
#ifdef _WIN32
				int32_t length = WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, nullptr, 0, nullptr, nullptr);
				if (length > 0)
				{
					to.resize(length - 1);
					WideCharToMultiByte(CP_UTF8, 0, from.c_str(), -1, to.data(), length, nullptr, nullptr);
				}
#endif
			}

			static void convert_string(const std::string& from, std::wstring& to)
			{
#ifdef _WIN32
				int32_t length = MultiByteToWideChar(CP_UTF8, 0, from.c_str(), -1, nullptr, 0);
				if (length > 0)
				{
					to.resize(length);
					MultiByteToWideChar(CP_UTF8, 0, from.c_str(), -1, to.data(), length);
				}
#endif
			}

			template<typename T>
			static void hash_combine(uint16_t& seed, const T& val)
			{
				seed ^= std::hash<T>()(val) + 0x9e37U + (seed << 3) + (seed >> 1);
			}

			template<typename T>
			static void hash_combine(uint32_t& seed, const T& val)
			{
				seed ^= std::hash<T>()(val) + 0x9e3779b9U + (seed << 6) + (seed >> 2);
			}

			template<typename T>
			static void hash_combine(uint64_t& seed, const T& val)
			{
				seed ^= std::hash<T>()(val) + 0x9e3779b97f4a7c15LLU  + (seed << 12) + (seed >> 4);
			}

			static std::string get_current_date_time()
			{
				constexpr const char* UNDEFINED_TIME = "UNDEFINED_TIME";
				
				std::time_t now = std::time(0);
				std::tm time;
				auto errorCode = localtime_s(&time, &now);
				if (errorCode)
				{
					LOG_ERROR("CoreUtils::get_current_date_time(): Failed to get date and time using localtime_s. {} will be returned.", UNDEFINED_TIME)
					return UNDEFINED_TIME;
				}
				
				char date[80];
				if (std::strftime(date, sizeof(date), "%d-%m-%Y_%H-%M-%S", &time))
					return date;

				LOG_ERROR("CoreUtils::get_current_date_time(): Failed to format date and time using strftime. {} will be returned.", UNDEFINED_TIME)
				return UNDEFINED_TIME;
			}

			template<typename Enum>
			static constexpr typename std::underlying_type<Enum>::type to_underlying(Enum e)
			{
				return static_cast<typename std::underlying_type<Enum>::type>(e);
			}
	};
}
