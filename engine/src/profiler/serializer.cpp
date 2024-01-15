#include "serializer.h"
#include "logger.h"
#include "file_system/utils.h"
#include "core/utils.h"
#include <json.hpp>

using namespace ad_astris::profiler;

constexpr const char* FRAME_STATS_FILE_EXTENSION = "aaframestats";

void Serializer::init(const ProfilerInstance* profilerInstance, io::FileSystem* fileSystem)
{
	assert(profilerInstance && fileSystem);
	_profilerInstance = profilerInstance;
	_fileSystem = fileSystem;
}

void Serializer::save_frame_stats_file(FrameStats* frameStats)
{
	std::string serializedMetadata;
	frameStats->serialize(serializedMetadata);

	io::URI frameStatsFilePath{
		fmt::format(
			"{}/intermediate/profiler_stats/{}.{}",
			_fileSystem->get_project_root_path().c_str(),
			frameStats->get_name(),
			FRAME_STATS_FILE_EXTENSION) };
	
	io::Utils::write_file(_fileSystem, frameStatsFilePath, serializedMetadata);
	LOG_INFO("profiler::Serializer::save_frame_stats_file(): Saved frame stats file {}", frameStats->get_name())
}

void Serializer::read_frame_stats_file(const io::URI& frameStatsFilePath)
{
	std::string extension = io::Utils::get_file_extension(frameStatsFilePath);
	if (extension != FRAME_STATS_FILE_EXTENSION)
	{
		LOG_ERROR("profiler::Serializer::read_frame_stats_file(): File has extension {} not {}", extension, FRAME_STATS_FILE_EXTENSION)
		return;
	}

	std::string serializedMetadata;
	io::Utils::read_file(_fileSystem, frameStatsFilePath, serializedMetadata);
	_profilerInstance->get_frame_stats_manager().build_frame_stats(serializedMetadata);
}

bool Serializer::is_initialized()
{
	return _profilerInstance && _fileSystem;
}
