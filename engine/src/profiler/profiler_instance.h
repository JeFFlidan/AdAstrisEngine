#pragma once

#include "statistics.h"
#include "core/pool_allocator.h"
#include "file_system/file_system.h"
#include "rhi/engine_rhi.h"

namespace ad_astris::profiler
{
	constexpr uint32_t MAX_FRAME_COUNT = 120; 
	
	struct ProfilerInstanceInitContext
	{
		bool isEnabled{ true };
	};

	using RangeID = size_t;
	
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
			[[nodiscard]] const Statistics& get_statistics() const { return _statistics; }
			void set_rhi(rhi::IEngineRHI* rhi) { _rhi = rhi; }
			void set_enable(bool isEnabled) { _isEnabled = isEnabled; }
			void set_max_frame_count(uint32_t maxFrameCount) { _maxFrameCount = maxFrameCount; }
		
		private:
			rhi::IEngineRHI* _rhi{ nullptr };
			rhi::CommandBuffer _profilerCmd;
			std::vector<rhi::Buffer> _queryResultBuffers;
			rhi::QueryPool _timestampQueryPool;
			rhi::QueryPool _pipelineStatisticsQueryPool;
			std::atomic<uint32_t> _nextTimestampQuery{ 0 };
			std::atomic<uint32_t> _nextPipelineStatisticsQuery{ 0 };
		
			Statistics _statistics;
		
			PoolAllocator<CPURange> _cpuRangePool;
			PoolAllocator<GPURange> _gpuRangePool;
			std::vector<CPURange*> _activeCPURanges;
			std::vector<GPURange*> _activeGPURanges;
			std::mutex _cpuRangeMutex;
			std::mutex _gpuRangeMutex;
			RangeID _cpuFrame{ 0 };
			RangeID _gpuFrame{ 0 };
		
			uint32_t _maxFrameCount{ MAX_FRAME_COUNT };
			uint32_t _currentFrame{ 0 };
		
			bool _isEnabled{ true };
			bool _isInitialized{ false };

			void init();
			const rhi::Buffer& get_current_result_buffer();
	};
}
