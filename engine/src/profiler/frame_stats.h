#pragma once

#include "types.h"
#include "rhi/engine_rhi.h"
#include <unordered_map>

namespace ad_astris::profiler
{
	class FrameStats
	{
		public:
			FrameStats() = default;
			FrameStats(FrameID frameID);

			void add_range(CPURange* cpuRange);
			void add_range(GPURange* gpuRange);
			void calculate_memory_usage(rhi::RHI* rhi);

			void serialize(std::string& outputMetadata);
			void deserialize(std::string& inputMetadata);

			void reset(FrameID frameID);

			const std::unordered_map<RangeName, Timing>& get_cpu_timings() const
			{
				return _cpuRangeTimingByRangeName;
			}

			const std::unordered_map<RangeName, Timing>& get_gpu_timings() const
			{
				return _gpuRangeTimingByRangeName;
			}

			const CPUMemoryUsage& get_cpu_memory_usage() const
			{
				return _cpuMemoryUsage;
			}

			const rhi::GPUMemoryUsage& get_gpu_memory_usage() const
			{
				return _gpuMemoryUsage;
			}

			FrameID get_id() const
			{
				return _frameID;
			}
		
			FrameName get_name() const
			{
				return _frameName;
			}
		
			void set_name(FrameName frameName)
			{
				_frameName = frameName;
			}
		
		private:
			FrameID _frameID;
			FrameName _frameName;
			std::unordered_map<RangeName, Timing> _cpuRangeTimingByRangeName;
			std::unordered_map<RangeName, Timing> _gpuRangeTimingByRangeName;
			CPUMemoryUsage _cpuMemoryUsage;
			rhi::GPUMemoryUsage _gpuMemoryUsage;

			void generate_frame_name();
	};
}