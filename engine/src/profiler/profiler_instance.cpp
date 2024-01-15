#include "profiler_instance.h"

using namespace ad_astris;
using namespace profiler;

constexpr uint32_t RANGE_COUNT = 512;

ProfilerInstance::ProfilerInstance(ProfilerInstanceInitContext& initContext)
{
	_isEnabled = initContext.isEnabled;
	_frameStatsManager = std::make_unique<FrameStatsManager>(initContext.frameStatsHistoryCapacity);

	if (!_isEnabled)
		return;
}

ProfilerInstance::~ProfilerInstance()
{
	_cpuRangePool.cleanup();
	_gpuRangePool.cleanup();
}

void ProfilerInstance::begin_cpu_frame()
{
	if (!_isEnabled)
		return;

	if (!_isInitialized)
		init();

	_cpuFrame = begin_cpu_range("CPU Frame");
	_activeFrameStats = _frameStatsManager->allocate_frame_stats(_currentFrameID);
}

void ProfilerInstance::begin_gpu_frame()
{
	if (!_isEnabled)
		return;

	if (!_isInitialized)
		init();

	_rhi->begin_command_buffer(&_profilerCmd);
	_rhi->reset_query(&_profilerCmd, &_timestampQueryPool, 0, _timestampQueryPool.info.queryCount);
	//_rhi->reset_query(&_profilerCmd, &_pipelineStatisticsQueryPool, 0, _pipelineStatisticsQueryPool.info.queryCount);
	_gpuFrame = begin_gpu_range("GPU range", _profilerCmd);
}

void ProfilerInstance::end_gpu_frame()
{
	if (!_isEnabled)
		return;
	
	GPURange* gpuFrameRange = _activeGPURanges[_gpuFrame];
	gpuFrameRange->endTimeQueryIndex = _nextTimestampQuery.fetch_add(1);
	GPURange* lastGpuRange = _activeGPURanges.back();
	_rhi->end_query(&lastGpuRange->cmd, &_timestampQueryPool, gpuFrameRange->endTimeQueryIndex);
	const rhi::Buffer& buffer = get_current_result_buffer();
	_rhi->copy_query_pool_results(&lastGpuRange->cmd, &_timestampQueryPool, 0, _nextTimestampQuery.load(), sizeof(uint64_t), &buffer);
}

RangeID ProfilerInstance::begin_gpu_range(const std::string& rangeName, const rhi::CommandBuffer& cmd)
{
	if (!_isEnabled)
		return 0;

	std::scoped_lock<std::mutex> locker(_gpuRangeMutex);
	GPURange* range = _gpuRangePool.allocate();
	range->name = rangeName;
	range->cmd = cmd;
	range->beginTimeQueryIndex = _nextTimestampQuery.fetch_add(1);
	_rhi->end_query(&cmd, &_timestampQueryPool, range->beginTimeQueryIndex);

	RangeID rangeID = _activeGPURanges.size();
	_activeGPURanges.push_back(range);
	
	return rangeID;
}

void ProfilerInstance::end_frame()
{
	if (!_isEnabled)
		return;

	_activeFrameStats->calculate_memory_usage(_rhi);
	
	end_cpu_range(_cpuFrame);
	_activeCPURanges.clear();

	++_currentFrameID;
	
	const uint64_t* queryResults = (const uint64_t*)get_current_result_buffer().mappedData;
	double timestampFrequence = (double)_rhi->get_timestamp_frequency() / 1000.0;
	
	for (auto& range : _activeGPURanges)
	{
		const uint64_t beginTime = queryResults[range->beginTimeQueryIndex];
		const uint64_t endTime = queryResults[range->endTimeQueryIndex];
		range->time = (float)abs((double)(endTime - beginTime) / timestampFrequence);
		_activeFrameStats->add_range(range);
		_gpuRangePool.free(range);
	}

	_activeGPURanges.clear();
	
	_nextTimestampQuery.store(0);
	_nextPipelineStatisticsQuery.store(0);
}

void ProfilerInstance::end_gpu_range(RangeID rangeID)
{
	if (!_isEnabled)
		return;

	std::scoped_lock<std::mutex> locker(_gpuRangeMutex);
	if (rangeID > _activeGPURanges.size())
	{
		LOG_ERROR("ProfilerInstance::end_gpu_range(): Invalid gpu range id {}", rangeID)
		return;
	}

	GPURange* range = _activeGPURanges[rangeID];
	if (range->isFinished)
	{
		LOG_ERROR("ProfilerInstance::end_gpu_range(): GPU range {} has been already finished", range->name)
		return;
	}

	range->endTimeQueryIndex = _nextTimestampQuery.fetch_add(1);
	_rhi->end_query(&range->cmd, &_timestampQueryPool, range->endTimeQueryIndex);
}

RangeID ProfilerInstance::begin_cpu_range(const std::string& rangeName)
{
	if (!_isEnabled)
		return 0;

	std::scoped_lock<std::mutex> locker(_cpuRangeMutex);
	CPURange* range = _cpuRangePool.allocate();
	range->name = rangeName;
	range->timer.record();
	
	RangeID rangeID = _activeCPURanges.size();
	_activeCPURanges.push_back(range);
	return rangeID;
}

void ProfilerInstance::end_cpu_range(RangeID rangeID)
{
	if (!_isEnabled)
		return;

	if (rangeID > _activeCPURanges.size())
	{
		LOG_ERROR("ProfilerInstance::end_cpu_range(): Invalid cpu range id {}", rangeID)
		return;
	}

	CPURange* range = _activeCPURanges[rangeID];
	if (range->isFinished)
	{
		LOG_ERROR("ProfilerInstance::end_cpu_range(): CPU range {} has been already finished", range->name)
		return;
	}

	range->time = range->timer.elapsed_milliseconds();
	range->isFinished = true;

	std::scoped_lock<std::mutex> locker(_cpuRangeMutex);
	_activeFrameStats->add_range(range);
	_cpuRangePool.free(range);
}

void ProfilerInstance::start_collecting_pipeline_statistics(const std::string& pipelineName, const rhi::CommandBuffer& cmd)
{
	if (!_isEnabled)
		return;
}

void ProfilerInstance::finish_collecting_pipeline_statistics()
{
	if (!_isEnabled)
		return;
}

void ProfilerInstance::init()
{
	if (_isInitialized)
		return;
	
	_timestampQueryPool.info.type = rhi::QueryType::TIMESTAMP;
	_timestampQueryPool.info.queryCount = RANGE_COUNT;
	_rhi->create_query_pool(&_timestampQueryPool);

	_pipelineStatisticsQueryPool.info.type = rhi::QueryType::PIPELINE_STATISTICS;
	_pipelineStatisticsQueryPool.info.queryCount = RANGE_COUNT;
	_rhi->create_query_pool(&_pipelineStatisticsQueryPool);

	_cpuRangePool.allocate_new_pool(RANGE_COUNT);
	_gpuRangePool.allocate_new_pool(RANGE_COUNT);

	uint32_t bufferCount = _rhi->get_buffer_count();
	for (auto i = 0; i != bufferCount; ++i)
	{
		rhi::Buffer& buffer = _queryResultBuffers.emplace_back();
		buffer.bufferInfo.size = sizeof(uint64_t) * RANGE_COUNT;
		buffer.bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_DST;
		buffer.bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
		_rhi->create_buffer(&buffer);
	}

	_isInitialized = true;
}

const rhi::Buffer& ProfilerInstance::get_current_result_buffer()
{
	return _queryResultBuffers[_currentFrameID % _rhi->get_buffer_count()];
}
