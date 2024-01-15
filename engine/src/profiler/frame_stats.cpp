#include "frame_stats.h"
#include "logger.h"
#include "file_system/file.h"
#include "core/utils.h"
#include <json.hpp>
#include <fmt/format.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

using namespace ad_astris;
using namespace profiler;
using namespace nlohmann;

constexpr const char* CPU_RANGES_KEY = "cpu_ranges";
constexpr const char* GPU_RANGES_KEY = "gpu_ranges";
constexpr const char* FRAME_NAME_KEY = "frame_name";
constexpr const char* FRAME_ID_KEY = "frame_id";
constexpr const char* CPU_TOTAL_PHYSICAL_MEMORY_KEY = "cpu_total_physical_memory";
constexpr const char* CPU_TOTAL_VIRTUAL_MEMORY_KEY = "cpu_total_virtual_memory";
constexpr const char* CPU_PROCESSED_PHYSICAL_MEMORY_KEY = "cpu_processed_physical_memory";
constexpr const char* CPU_PROCESSED_VIRTUAL_MEMORY_KEY = "cpu_processed_virtual_memory";
constexpr const char* GPU_TOTAL_MEMORY_KEY = "gpu_total_memory";
constexpr const char* GPU_USAGE_KEY = "gpu_usage";

FrameStats::FrameStats(FrameID frameID) : _frameID(frameID)
{
	generate_frame_name();
}

void FrameStats::add_range(CPURange* cpuRange)
{
	auto it = _cpuRangeTimingByRangeName.find(cpuRange->name);
	if (it != _cpuRangeTimingByRangeName.end())
	{
		LOG_WARNING("FrameStats::add_range(): CPU range with name {} exists", cpuRange->name)
		return;
	}

	_cpuRangeTimingByRangeName[cpuRange->name] = cpuRange->time;
}

void FrameStats::add_range(GPURange* gpuRange)
{
	auto it = _cpuRangeTimingByRangeName.find(gpuRange->name);
	if (it != _cpuRangeTimingByRangeName.end())
	{
		LOG_WARNING("FrameStats::add_range(): GPU range with name {} exists", gpuRange->name)
		return;
	}

	_gpuRangeTimingByRangeName[gpuRange->name] = gpuRange->time;
}

void FrameStats::calculate_memory_usage(rhi::IEngineRHI* rhi)
{
	_gpuMemoryUsage = rhi->get_memory_usage();

	// https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process
#ifdef _WIN32
	MEMORYSTATUSEX memoryInfo{};
	memoryInfo.dwLength = sizeof(MEMORYSTATUSEX);
	BOOL result = GlobalMemoryStatusEx(&memoryInfo);
	assert(result);
	_cpuMemoryUsage.totalPhysical = memoryInfo.ullTotalPhys;
	_cpuMemoryUsage.totalVirtual = memoryInfo.ullTotalVirtual;

	PROCESS_MEMORY_COUNTERS_EX processMemory{};
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&processMemory, sizeof(processMemory));
	_cpuMemoryUsage.processedPhysical += processMemory.WorkingSetSize;
	_cpuMemoryUsage.processedVirtual += processMemory.PrivateUsage;
#endif
}

void FrameStats::serialize(std::string& outputMetadata)
{
	json cpuRangesJson;
	for (auto& pair : _cpuRangeTimingByRangeName)
	{
		cpuRangesJson[pair.first] = pair.second;
	}

	json gpuRangesJson;
	for (auto& pair : _gpuRangeTimingByRangeName)
	{
		gpuRangesJson[pair.first] = pair.second;
	}
	
	json frameStatsJson;
	frameStatsJson[CPU_RANGES_KEY] = cpuRangesJson;
	frameStatsJson[GPU_RANGES_KEY] = gpuRangesJson;
	frameStatsJson[FRAME_NAME_KEY] = _frameName;
	frameStatsJson[FRAME_ID_KEY] = _frameID;
	frameStatsJson[CPU_TOTAL_PHYSICAL_MEMORY_KEY] = _cpuMemoryUsage.totalPhysical;
	frameStatsJson[CPU_TOTAL_VIRTUAL_MEMORY_KEY] = _cpuMemoryUsage.totalVirtual;
	frameStatsJson[CPU_PROCESSED_PHYSICAL_MEMORY_KEY] = _cpuMemoryUsage.processedPhysical;
	frameStatsJson[CPU_PROCESSED_VIRTUAL_MEMORY_KEY] = _cpuMemoryUsage.processedVirtual;
	frameStatsJson[GPU_TOTAL_MEMORY_KEY] = _gpuMemoryUsage.total;
	frameStatsJson[GPU_USAGE_KEY] = _gpuMemoryUsage.usage;

	outputMetadata = frameStatsJson.dump(4);
}

void FrameStats::deserialize(std::string& inputMetadata)
{
	json frameStatsJson = json::parse(inputMetadata);
	_frameName = frameStatsJson[FRAME_NAME_KEY];
	_frameID = frameStatsJson[FRAME_ID_KEY];
	_cpuMemoryUsage.totalPhysical = frameStatsJson[CPU_TOTAL_PHYSICAL_MEMORY_KEY];
	_cpuMemoryUsage.totalVirtual = frameStatsJson[CPU_TOTAL_VIRTUAL_MEMORY_KEY];
	_cpuMemoryUsage.processedPhysical = frameStatsJson[CPU_PROCESSED_PHYSICAL_MEMORY_KEY];
	_cpuMemoryUsage.processedVirtual = frameStatsJson[CPU_PROCESSED_VIRTUAL_MEMORY_KEY];
	_gpuMemoryUsage.total = frameStatsJson[GPU_TOTAL_MEMORY_KEY];
	_gpuMemoryUsage.usage = frameStatsJson[GPU_USAGE_KEY];
	json cpuRangesJson = frameStatsJson[CPU_RANGES_KEY];
	json gpuRangesJson = frameStatsJson[GPU_RANGES_KEY];
	for (auto& keyValue : cpuRangesJson.items())
	{
		_cpuRangeTimingByRangeName[keyValue.key()] = keyValue.value();
	}

	for (auto& keyValue : gpuRangesJson.items())
	{
		_gpuRangeTimingByRangeName[keyValue.key()] = keyValue.value();
	}
}

void FrameStats::reset(FrameID frameID)
{
	_frameID = frameID;
	generate_frame_name();
	// Maybe I don't need this and I have to remove warnings in add_range methods.
	_cpuRangeTimingByRangeName.clear();
	_gpuRangeTimingByRangeName.clear();
}

void FrameStats::generate_frame_name()
{
	_frameName = fmt::format("Frame_{}_{}", _frameID, CoreUtils::get_current_date_time());
}
