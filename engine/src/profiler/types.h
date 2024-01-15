#pragma once

#include "core/timer.h"
#include "rhi/resources.h"
#include <thread>
#include <string>

namespace ad_astris::profiler
{
	using ThreadId = std::thread::id;
	using RangeID = size_t;
	using FrameID = size_t;
	using RangeName = std::string;
	using FrameName = std::string;
	using Timing = float;
	
	struct Range
	{
		RangeName name;
		float time{ 0 };
		bool isFinished{ false };
	};
	
	struct CPURange : public Range
	{
		Timer timer;
		ThreadId threadId;
	};
	
	struct GPURange : public Range
	{
		rhi::CommandBuffer cmd;
		int32_t beginTimeQueryIndex{ 0 };
		int32_t endTimeQueryIndex{ 0 };
	};

	struct CPUMemoryUsage
	{
		uint64_t totalPhysical{ 0 };
		uint64_t totalVirtual{ 0 };
		uint64_t processedPhysical{ 0 };
		uint64_t processedVirtual{ 0 };
	};
}