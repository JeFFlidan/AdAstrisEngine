#pragma once

#include <chrono>

namespace ad_astris
{
	// Took from https://github.com/turanszkij/WickedEngine/blob/b07d3a73ead37000249b4a0cde67ed256aaac961/WickedEngine/wiTimer.h
	class Timer
	{
		public:
			void record()
			{
				time = std::chrono::high_resolution_clock::now();
			}

			double record_elapsed_seconds()
			{
				auto time2 = std::chrono::high_resolution_clock::now();
				auto elapsed = elapsed_seconds_since(time2);
				time = time2;
				return elapsed;
			}

			double elapsed_seconds_since(std::chrono::high_resolution_clock::time_point time2)
			{
				std::chrono::duration<double> timeSpan = std::chrono::duration_cast<std::chrono::duration<double>>(time2 - time);
				return timeSpan.count();
			}

			double elapsed_seconds()
			{
				return elapsed_seconds_since(std::chrono::high_resolution_clock::now());
			}

			double elapsed_milliseconds()
			{
				return elapsed_seconds() * 1000.0f;
			}

		private:
			std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();
	};
}