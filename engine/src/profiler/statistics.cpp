#include "statistics.h"

using namespace ad_astris::profiler;

void Statistics::add_range(CPURange* cpuRange)
{
	auto it = _cpuRangeTimingsByName.find(cpuRange->name);
	if (it == _cpuRangeTimingsByName.end())
	{
		_cpuRangeTimingsByName.emplace(cpuRange->name, Timings());
	}

	Timings& timings = _cpuRangeTimingsByName[cpuRange->name];
	if (timings.size() != _maxFrameCount)
	{
		timings.resize(_maxFrameCount);
	}
	timings[_currentFrame % _maxFrameCount] = cpuRange->time;
}

void Statistics::add_range(GPURange* gpuRange)
{
	auto it = _gpuRangeTimingsByName.find(gpuRange->name);
	if (it == _gpuRangeTimingsByName.end())
	{
		_gpuRangeTimingsByName.emplace(gpuRange->name, Timings());
	}

	Timings& timings = _gpuRangeTimingsByName[gpuRange->name];
	if (timings.size() != _maxFrameCount)
	{
		timings.resize(_maxFrameCount);
	}
	timings[_currentFrame % _maxFrameCount] = gpuRange->time;
}
