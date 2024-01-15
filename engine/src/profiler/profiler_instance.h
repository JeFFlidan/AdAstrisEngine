#pragma once

#include "frame_stats_manager.h"
#include "core/pool_allocator.h"
#include "file_system/file_system.h"
#include "rhi/engine_rhi.h"

namespace ad_astris::profiler
{
	constexpr uint32_t MAX_FRAME_COUNT = 120; 
	
	struct ProfilerInstanceInitContext
	{
		bool isEnabled{ true };
		uint64_t frameStatsHistoryCapacity{ FRAME_STATS_HISTORY_CAPACITY };
	};
	
	class ProfilerInstance
	{
		public:
			ProfilerInstance(ProfilerInstanceInitContext& initContext);
			~ProfilerInstance();

			void begin_cpu_frame();
			void begin_gpu_frame();
			void end_gpu_frame();
			void end_frame();

			[[nodiscard]] RangeID begin_gpu_range(const std::string& rangeName, const rhi::CommandBuffer& cmd);
			void end_gpu_range(RangeID);
			[[nodiscard]] RangeID begin_cpu_range(const std::string& rangeName);
			void end_cpu_range(RangeID);

			void start_collecting_pipeline_statistics(const std::string& pipelineName, const rhi::CommandBuffer& cmd);
			void finish_collecting_pipeline_statistics();
		
			[[nodiscard]] bool is_enabled() const { return _isEnabled; }
			[[nodiscard]] FrameStatsManager& get_frame_stats_manager() const { return *_frameStatsManager; }
			void set_rhi(rhi::IEngineRHI* rhi) { _rhi = rhi; }
			void set_enable(bool isEnabled) { _isEnabled = isEnabled; }
		
		private:
			rhi::IEngineRHI* _rhi{ nullptr };
			rhi::CommandBuffer _profilerCmd;
			std::vector<rhi::Buffer> _queryResultBuffers;
			rhi::QueryPool _timestampQueryPool;
			rhi::QueryPool _pipelineStatisticsQueryPool;
			std::atomic<uint32_t> _nextTimestampQuery{ 0 };
			std::atomic<uint32_t> _nextPipelineStatisticsQuery{ 0 };
		
			std::unique_ptr<FrameStatsManager> _frameStatsManager{ nullptr };

			FrameStats* _activeFrameStats{ nullptr };
			PoolAllocator<CPURange> _cpuRangePool;
			PoolAllocator<GPURange> _gpuRangePool;
			std::vector<CPURange*> _activeCPURanges;
			std::vector<GPURange*> _activeGPURanges;
			std::mutex _cpuRangeMutex;
			std::mutex _gpuRangeMutex;
			RangeID _cpuFrame{ 0 };
			RangeID _gpuFrame{ 0 };
		
			FrameID _currentFrameID{ 0 };
		
			bool _isEnabled{ true };
			bool _isInitialized{ false };

			void init();
			const rhi::Buffer& get_current_result_buffer();
	};
}
