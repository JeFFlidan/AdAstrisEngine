#pragma once

#include "profiler_instance.h"

namespace ad_astris::profiler
{
	class Profiler
	{
		public:
			static void init(ProfilerInstance* profilerInstance)
			{
				assert(profilerInstance);
				_profilerInstance = profilerInstance;
			}

			static void set_enable(bool isEnabled)
			{
				if (_profilerInstance)
					_profilerInstance->set_enable(isEnabled);
			}

			static void begin_cpu_frame()
			{
				if (_profilerInstance)
					_profilerInstance->begin_cpu_frame();
			}

			static void begin_gpu_frame()
			{
				if (_profilerInstance)
					_profilerInstance->begin_gpu_frame();
			}

			static void end_gpu_frame()
			{
				if (_profilerInstance)
					_profilerInstance->end_gpu_frame();
			}

			static void end_frame()
			{
				if (_profilerInstance)
					_profilerInstance->end_frame();
			}

			[[nodiscard]] static RangeID begin_gpu_range(const std::string& rangeName, rhi::CommandBuffer& cmd)
			{
				if (_profilerInstance)
					return _profilerInstance->begin_gpu_range(rangeName, cmd);

				return 0;
			}

			static void end_gpu_range(RangeID rangeID)
			{
				if (_profilerInstance)
					_profilerInstance->end_gpu_range(rangeID);
			}

			[[nodiscard]] static RangeID begin_cpu_range(const std::string& rangeName)
			{
				if (_profilerInstance)
					return _profilerInstance->begin_cpu_range(rangeName);

				return 0;
			}

			static void end_cpu_range(RangeID rangeID)
			{
				if (_profilerInstance)
					_profilerInstance->end_cpu_range(rangeID);
			}

			static void start_collecting_pipeline_statistics(const std::string& pipelineName, rhi::CommandBuffer& cmd)
			{
				if (_profilerInstance)
					_profilerInstance->start_collecting_pipeline_statistics(pipelineName, cmd);
			}

			static void finish_collecting_pipeline_statistics()
			{
				if (_profilerInstance)
					_profilerInstance->finish_collecting_pipeline_statistics();
			}

			static void set_frame_name(FrameID frameID, const FrameName& frameName)
			{
				const FrameStatsManager& frameStatsManager = _profilerInstance->get_frame_stats_manager();
				frameStatsManager.get_frame_stats(frameID)->set_name(frameName);
			}

			// Returns a copy of FrameStats object
			static std::unique_ptr<FrameStats> get_frame_stats(FrameID frameID)
			{
				FrameStatsManager& frameStatsManager = _profilerInstance->get_frame_stats_manager();
				return std::make_unique<FrameStats>(*frameStatsManager.get_frame_stats(frameID));
			}

			static ProfilerInstance* get_profiler_instance()
			{
				return _profilerInstance;
			}

		private:
			inline static ProfilerInstance* _profilerInstance{ nullptr };
	};
}