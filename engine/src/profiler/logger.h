#pragma once

#include <string_view>
#include <fmt/os.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <chrono>
#include <fmt/chrono.h>

#define LOG_FATAL_ENABLED 1
#define LOG_ERROR_ENABLED 1
#define LOG_WARNING_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_INFO_SUCCESS 1

#ifdef VK_RELEASE
#define LOG_FATAL_ENABLED 0
#define LOG_ERROR_ENABLED 0
#define LOG_WARNING_ENABLED 0
#define LOG_INFO_ENABLED 0
#define LOG_INFO_SUCCESS 0
#endif

#if LOG_FATAL_ENABLED == 1
#define LOG_FATAL(message,...) Logger::get_logger().log(LogType::FATAL, message, ##__VA_ARGS__);
#else
#define LOG_FATAL(message,...)
#endif

#if LOG_ERROR_ENABLED == 1
#define LOG_ERROR(message,...) Logger::get_logger().log(LogType::ERROR_TYPE, message, ##__VA_ARGS__);
#else
#define LOG_ERROR(message,...)
#endif

#if LOG_WARNING_ENABLED == 1
#define LOG_WARNING(message,...) Logger::get_logger().log(LogType::WARNING, message, ##__VA_ARGS__);
#else
#define LOG_WARNING(message,...)
#endif

#if LOG_INFO_ENABLED == 1
#define LOG_INFO(message,...) Logger::get_logger().log(LogType::INFO, message, ##__VA_ARGS__);
#else
#define LOG_INFO(message,...)
#endif

#if LOG_INFO_SUCCESS == 1
#define LOG_SUCCESS(message,...) Logger::get_logger().log(LogType::SUCCESS, message, ##__VA_ARGS__);
#else
#define LOG_SUCCESS(message,...)
#endif

enum class LogType
{
	FATAL,
	ERROR_TYPE,
	WARNING,
	INFO,
	SUCCESS
};

class Logger
{
	public:
		template <typename... Args>
		static void print(std::string_view message, Args... args)
		{
			fmt::print((message), args...);
			fmt::print("\n");
		}

		template <typename... Args>
		static void log(LogType logType, std::string_view message, Args... args)
		{
			print_time();

			switch (logType)
			{
				case(LogType::FATAL):
					fmt::print(fg(fmt::color::dark_red) | fmt::emphasis::bold, "[FATAL]:   ");
					break;
				case(LogType::ERROR_TYPE):
					fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "[ERROR]:   ");
					break;
				case(LogType::WARNING):
					fmt::print(fg(fmt::color::dark_violet) | fmt::emphasis::bold, "[WARNING]: ");
					break;
				case(LogType::INFO):
					fmt::print(fg(fmt::color::royal_blue) | fmt::emphasis::bold, "[INFO]:    ");
					break;
				case(LogType::SUCCESS):
					fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold, "[SUCCESS]: ");
					break;
			}

			print(message, args...);

			if (logType == LogType::FATAL)
				abort();
		}

		static Logger& get_logger()
		{
			static Logger logger{};
			return logger;
		}

		static void print_time()
		{
			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			auto time_point = now - Logger::get_logger().start_time;
			return fmt::print("[{:%M:%S}]", time_point);
		}

		std::chrono::time_point<std::chrono::system_clock> start_time;
};
