#include "frame_stats_manager.h"

using namespace ad_astris;
using namespace profiler;

constexpr uint64_t LOADED_FRAME_STATS_CAPACITY = 50;

FrameStatsManager::FrameStatsManager(uint64_t frameStatsHistoryCapacity)
	: _frameStatsHistoryCapacity(frameStatsHistoryCapacity)
{
	_frameStatsHistory.resize(_frameStatsHistoryCapacity);
	_frameStatsPool.allocate_new_pool(_frameStatsHistoryCapacity + LOADED_FRAME_STATS_CAPACITY);
}

FrameStatsManager::~FrameStatsManager()
{
	_frameStatsPool.cleanup();
}

FrameStats* FrameStatsManager::allocate_frame_stats(FrameID currentFrameID)
{
	if (currentFrameID < _frameStatsHistoryCapacity)
	{
		_frameStatsHistory[currentFrameID] = _frameStatsPool.allocate(currentFrameID);
		return _frameStatsHistory[currentFrameID];
	}
	
	FrameStats* frameStats = _frameStatsHistory[currentFrameID % _frameStatsHistoryCapacity];
	frameStats->reset(currentFrameID);
	return frameStats;
}

FrameStats* FrameStatsManager::build_frame_stats(std::string& inputSerializedMetadata)
{
	FrameStats* frameStats = _frameStatsPool.allocate();
	frameStats->deserialize(inputSerializedMetadata);

	_loadedFrameStatsByFrameName[frameStats->get_name()] = frameStats;

	size_t loadedFrameStatsCount = _loadedFrameStatsByFrameName.size(); 
	if (loadedFrameStatsCount % LOADED_FRAME_STATS_CAPACITY == 0)
	{
		_frameStatsPool.allocate(LOADED_FRAME_STATS_CAPACITY);
	}

	return frameStats;
}
