#include "serializer.h"
#include "file_system/utils.h"
#include "core/utils.h"
#include <json.hpp>

using namespace ad_astris::profiler;

void Serializer::init(const ProfilerInstance* profilerInstance, io::FileSystem* fileSystem)
{
	assert(profilerInstance && fileSystem);
	_profilerInstance = profilerInstance;
	_fileSystem = fileSystem;
}

void Serializer::serialize(const std::string& statisticsFileName)
{
	if (!is_initialized())
	{
		LOG_WARNING("profiler::Serializer::serialize(): Profiler is not initialized")
		return;
	}

	const Statistics& statistics = _profilerInstance->get_statistics();

	auto& cpuRangeTimings = statistics.get_cpu_range_timings();
	nlohmann::json cpuRangesJson;
	
	for (auto& pair : cpuRangeTimings)
	{
		const RangeName& rangeName = pair.first;
		const Timings& timings = pair.second;
		float avgTime = std::accumulate(timings.begin(), timings.end(), 0.0f) / timings.size();
		cpuRangesJson[rangeName] = avgTime;
	}
	
	auto& gpuRangeTimings = statistics.get_gpu_range_timings();
	nlohmann::json gpuRangesJson;

	for (auto& pair : gpuRangeTimings)
	{
		const RangeName& rangeName = pair.first;
		const Timings& timings = pair.second;
		float avgTime = std::accumulate(timings.begin(), timings.end(), 0.0f) / timings.size();
		gpuRangesJson[rangeName] = avgTime;
	}

	nlohmann::json statisticsJson;
	statisticsJson["cpu_ranges"] = cpuRangesJson.dump();
	statisticsJson["gpu_ranges"] = gpuRangesJson.dump();

	io::URI statsFolderPath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), "intermediate/profiler_stats");
	if (!io::Utils::exists(statsFolderPath))
	{
		io::Utils::create_folders(_fileSystem, statsFolderPath);
	}

	std::string completeStatsFileName = statisticsFileName + "_" + CoreUtils::get_current_date_time() + ".aastats";
	io::URI statsFilePath = io::Utils::get_absolute_path_to_file(statsFolderPath, completeStatsFileName);
	std::string strStatistics = statisticsJson.dump();
	io::Utils::write_file(_fileSystem, statsFilePath, strStatistics.c_str(), strStatistics.size());
}

bool Serializer::is_initialized()
{
	return _profilerInstance && _fileSystem;
}
