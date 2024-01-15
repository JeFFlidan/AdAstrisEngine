#pragma once

#include "frame_stats.h"
#include "core/pool_allocator.h"

namespace ad_astris::profiler
{
	constexpr uint64_t FRAME_STATS_HISTORY_CAPACITY = 50000;
	
	class FrameStatsManager
	{
		public:
			FrameStatsManager(uint64_t frameStatsHistoryCapacity);
			~FrameStatsManager();

			FrameStats* allocate_frame_stats(FrameID currentFrameID);
			FrameStats* build_frame_stats(std::string& inputSerializedMetadata);

			FrameStats* get_frame_stats(FrameID frameID) const
			{
				return _frameStatsHistory[frameID % _frameStatsHistoryCapacity];
			}
		
		private:
			ThreadSafePoolAllocator<FrameStats> _frameStatsPool;
			std::vector<FrameStats*> _frameStatsHistory;
			std::unordered_map<FrameName, FrameStats*> _loadedFrameStatsByFrameName;
			uint64_t _frameStatsHistoryCapacity{ 0 };
	};
}