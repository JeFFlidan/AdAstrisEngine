#pragma once

#include "rhi/resources.h"
#include "core/timer.h"
#include <map>

namespace ad_astris::profiler
{
	struct Range
	{
		std::string name;
		float time{ 0 };
		bool isFinished{ false };
	};
	
	struct CPURange : public Range
	{
		Timer timer;
	};
	
	struct GPURange : public Range
	{
		rhi::CommandBuffer cmd;
		int32_t beginTimeQueryIndex{ 0 };
		int32_t endTimeQueryIndex{ 0 };
	};
	
	struct MemoryUsage
	{
		
	};
	
	using RangeName = std::string;
	using Timings = std::vector<float>;

	class Statistics
	{
		public:
			void add_range(CPURange* cpuRange);
			void add_range(GPURange* gpuRange);

			[[nodiscard]] const std::map<RangeName, Timings>& get_cpu_range_timings() const
			{
				return _cpuRangeTimingsByName;
			}
		
			[[nodiscard]] const std::map<RangeName, Timings>& get_gpu_range_timings() const
			{
				return _gpuRangeTimingsByName;
			}

			void set_max_frame_count(uint32_t maxFrameCount) { _maxFrameCount = maxFrameCount; }
			void set_current_frame(uint32_t currentFrame) { _currentFrame = currentFrame; }
		
		private:
			std::map<RangeName, Timings> _cpuRangeTimingsByName;
			std::map<RangeName, Timings> _gpuRangeTimingsByName;
			uint32_t _maxFrameCount{ 0 };
			uint32_t _currentFrame{ 0 };
	};
}