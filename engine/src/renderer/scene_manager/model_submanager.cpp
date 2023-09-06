#include "model_submanager.h"
#include "engine_core/model/default_models.h"

using namespace ad_astris::renderer::impl;

ModelSubmanager::ModelSubmanager(SceneSubmanagerInitializationContext& initContext) : SceneSubmanagerBase(initContext)
{
	subscribe_to_events();
	ModelSubmanagerUpdatedEvent event;
	_eventManager->trigger_event(event);
}

void ModelSubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	if (!_areDefaultBuffersAllocated)
		allocate_default_buffers(cmdBuffer);

	allocate_staging_buffers(cmdBuffer);
	allocate_gpu_buffers(cmdBuffer);
}

void ModelSubmanager::reset_temp_arrays()
{
	_loadedModelsVertexArraySize_F32PNTC = 0;
	_loadedModelsIndexArraySize_F32PNTC = 0;
	_loadedStaticModelHandles.clear();
}

void ModelSubmanager::cleanup_staging_buffers()
{
	for (auto& buffer : _stagingBuffersToDelete)
		_rhi->destroy_buffer(buffer);
	_stagingBuffersToDelete.clear();
}

bool ModelSubmanager::need_allocation()
{
	return !_areGPUBuffersAllocated || !_loadedStaticModelHandles.empty();
}

void ModelSubmanager::subscribe_to_events()
{
	events::EventDelegate<resource::StaticModelLoadedEvent> staticModelLoadedDelegate = [&](resource::StaticModelLoadedEvent& event)
	{
		_loadedStaticModelHandles.push_back(event.get_model_handle());

		ecore::StaticModelData modelData = event.get_model_handle().get_resource()->get_model_data();
		switch (event.get_model_handle().get_resource()->get_vertex_format())
		{
			case ecore::model::VertexFormat::F32_PC:
			{
				break;
			}
			case ecore::model::VertexFormat::F32_PNTC:
			{
				_loadedModelsVertexArraySize_F32PNTC += modelData.vertexBufferSize;
				_loadedModelsIndexArraySize_F32PNTC += modelData.indexBufferSize;
				break;
			}
		}

		ModelSubmanagerUpdatedEvent updatedEvent;
		_eventManager->enqueue_event(updatedEvent);
	};

	_eventManager->subscribe(staticModelLoadedDelegate);
}

void ModelSubmanager::allocate_staging_buffers(rhi::CommandBuffer& cmdBuffer)
{
		if (_loadedStaticModelHandles.empty())
		return;
	
	LOG_INFO("MODELS COUNT: {}", _loadedStaticModelHandles.size())

	uint32_t vertexArrayOffset_F32PNTC = vertexArray_F32PNTC.size();
	uint32_t indexArrayOffset_F32PNTC = indexArray_F32PNTC.size();
	vertexArray_F32PNTC.resize(vertexArray_F32PNTC.size() + _loadedModelsVertexArraySize_F32PNTC);
	indexArray_F32PNTC.resize(indexArray_F32PNTC.size() + _loadedModelsIndexArraySize_F32PNTC);
	
	bool wereArraysChanged_F32PNTC = false;
	bool wereArraysChanged_F32PC = false;
	ecore::StaticModelData staticModelData;
	for (auto handle : _loadedStaticModelHandles)
	{
		staticModelData = handle.get_resource()->get_model_data();
		switch (handle.get_resource()->get_vertex_format())
		{
			case ecore::model::VertexFormat::F32_PNTC:
			{
				memcpy(
					vertexArray_F32PNTC.data() + vertexArrayOffset_F32PNTC,
					staticModelData.vertexBuffer,
					staticModelData.vertexBufferSize);
				vertexArrayOffset_F32PNTC += staticModelData.vertexBufferSize;
				
				memcpy(
					indexArray_F32PNTC.data() + indexArrayOffset_F32PNTC,
					staticModelData.indexBuffer,
					staticModelData.indexBufferSize);
				indexArrayOffset_F32PNTC += staticModelData.indexBufferSize;

				wereArraysChanged_F32PNTC = true;
				break;
			}
			case ecore::model::VertexFormat::F32_PC:
			{
				wereArraysChanged_F32PC = true;
				break;
			}
		}
	}

	if (wereArraysChanged_F32PNTC)
	{
		uint32_t fullVertexArraySize_F32PNTC = vertexArray_F32PNTC.size();
		uint32_t fullIndexArraySize_F32PNTC = indexArray_F32PNTC.size();
		uint32_t gpuVertexBufferSize_F32PNTC = _vertexBuffer_F32PNTC.buffer.size;
		uint32_t gpuIndexBufferSize_F32PNTC = _indexBuffer_F32PNTC.buffer.size;
		rhi::BufferInfo bufferInfo;
		bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
		bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;

		if (fullVertexArraySize_F32PNTC >= gpuVertexBufferSize_F32PNTC)
		{
			bufferInfo.size = fullVertexArraySize_F32PNTC;
			_rhi->create_buffer(&_stagingBuffers.vertexBuffer_F32PNTC, &bufferInfo, vertexArray_F32PNTC.data());
			LOG_INFO("ModelSubmanager::allocate_staging_buffers_with_all_models(): Allocated new staging vertex buffer F32PNTC with all models. Size: {}", _stagingBuffers.vertexBuffer_F32PNTC.size)
		}
		else
		{
			bufferInfo.size = _loadedModelsVertexArraySize_F32PNTC;
			uint32_t arrayOffset = vertexArray_F32PNTC.size() - _loadedModelsVertexArraySize_F32PNTC;
			_rhi->create_buffer(&_stagingBuffers.vertexBuffer_F32PNTC, &bufferInfo, vertexArray_F32PNTC.data() + arrayOffset);
			LOG_INFO("ModelSubmanager::allocate_staging_buffers_with_all_models(): Allocated new staging vertex buffer F32PNTC with recently loaded models. Size: {}", _stagingBuffers.vertexBuffer_F32PNTC.size)
		}

		if (fullIndexArraySize_F32PNTC >= gpuIndexBufferSize_F32PNTC)
		{
			bufferInfo.size = fullIndexArraySize_F32PNTC;
			_rhi->create_buffer(&_stagingBuffers.indexBuffer_F32PNTC, &bufferInfo, indexArray_F32PNTC.data());
			LOG_INFO("ModelSubmanager::allocate_staging_buffers_with_all_models(): Allocated new staging index buffer F32PNTC with all models. Size: {}", _stagingBuffers.indexBuffer_F32PNTC.size)
		}
		else
		{
			bufferInfo.size = _loadedModelsIndexArraySize_F32PNTC;
			uint32_t arrayOffset = indexArray_F32PNTC.size() - _loadedModelsIndexArraySize_F32PNTC;
			_rhi->create_buffer(&_stagingBuffers.indexBuffer_F32PNTC, &bufferInfo, indexArray_F32PNTC.data() + arrayOffset);
			LOG_INFO("ModelSubmanager::allocate_staging_buffers_with_all_models(): Allocated new staging index buffer F32PNTC with recently loaded models. Size: {}", _stagingBuffers.indexBuffer_F32PNTC.size)
		}
	
		_stagingBuffersToDelete.push_back(&_stagingBuffers.vertexBuffer_F32PNTC);
		_stagingBuffersToDelete.push_back(&_stagingBuffers.indexBuffer_F32PNTC);
	}

	if (wereArraysChanged_F32PC)
	{
		//TODO
	}
}

void ModelSubmanager::allocate_gpu_buffers(rhi::CommandBuffer& cmdBuffer)
{
	rhi::BufferInfo bufferInfo;
	bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
	
	if (!_loadedStaticModelHandles.empty())
	{
		uint64_t gpuBufferSize = 0, gpuBufferOffset = 0, fullStagingBufferSize = 0, stagingBufferOffset = 0;
		
		if (!_areGPUBuffersAllocated)
		{
			fullStagingBufferSize = _stagingBuffers.vertexBuffer_F32PNTC.size;
			gpuBufferSize = fullStagingBufferSize * 2 > DEFAULT_GPU_BUFFER_SIZE ? fullStagingBufferSize * 2 : DEFAULT_GPU_BUFFER_SIZE;
			bufferInfo.size = gpuBufferSize;
			bufferInfo.bufferUsage = rhi::ResourceUsage::VERTEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
			_rhi->create_buffer(&_vertexBuffer_F32PNTC.buffer, &bufferInfo);
			_rhi->copy_buffer(&cmdBuffer, &_stagingBuffers.vertexBuffer_F32PNTC, &_vertexBuffer_F32PNTC.buffer);
			_vertexBuffer_F32PNTC.offset = _stagingBuffers.vertexBuffer_F32PNTC.size;
			LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Allocated new vertex buffer F32PNTC. Size: {}. Offset: {}.", _vertexBuffer_F32PNTC.buffer.size, _vertexBuffer_F32PNTC.offset)

			fullStagingBufferSize = _stagingBuffers.indexBuffer_F32PNTC.size;
			gpuBufferSize = fullStagingBufferSize * 2 > DEFAULT_GPU_BUFFER_SIZE ? fullStagingBufferSize * 2 : DEFAULT_GPU_BUFFER_SIZE;
			bufferInfo.size = gpuBufferSize;
			bufferInfo.bufferUsage = rhi::ResourceUsage::INDEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
			_rhi->create_buffer(&_indexBuffer_F32PNTC.buffer, &bufferInfo);
			_rhi->copy_buffer(&cmdBuffer, &_stagingBuffers.indexBuffer_F32PNTC, &_indexBuffer_F32PNTC.buffer);
			_indexBuffer_F32PNTC.offset = _stagingBuffers.indexBuffer_F32PNTC.size;
			LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Allocated new vertex buffer F32PNTC. Size: {}. Offset: {}.", _indexBuffer_F32PNTC.buffer.size, _indexBuffer_F32PNTC.offset)
		}
		else
		{
			gpuBufferSize = _vertexBuffer_F32PNTC.buffer.size;
			fullStagingBufferSize = vertexArray_F32PNTC.size();
			if (fullStagingBufferSize >= gpuBufferSize)
			{
				_rhi->destroy_buffer(&_vertexBuffer_F32PNTC.buffer);
				bufferInfo.size = fullStagingBufferSize * 2;
				bufferInfo.bufferUsage = rhi::ResourceUsage::VERTEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
				_rhi->create_buffer(&_vertexBuffer_F32PNTC.buffer, &bufferInfo);
				_rhi->copy_buffer(&cmdBuffer, &_stagingBuffers.vertexBuffer_F32PNTC, &_vertexBuffer_F32PNTC.buffer);
				_vertexBuffer_F32PNTC.offset = _stagingBuffers.vertexBuffer_F32PNTC.size;
				LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Reallocated vertex buffer F32PNTC. Size: {}. Offset: {}", _vertexBuffer_F32PNTC.buffer.size, _vertexBuffer_F32PNTC.offset)
			}
			else
			{
				gpuBufferOffset = _vertexBuffer_F32PNTC.offset;
				_rhi->copy_buffer(
					&cmdBuffer,
					&_stagingBuffers.vertexBuffer_F32PNTC,
					&_vertexBuffer_F32PNTC.buffer,
					_stagingBuffers.vertexBuffer_F32PNTC.size,
					0,
					gpuBufferOffset);
				_vertexBuffer_F32PNTC.offset += _stagingBuffers.vertexBuffer_F32PNTC.size;
				LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Added info about new models to vertex buffer F32PNTC. Size: {}. Offset: {}", _vertexBuffer_F32PNTC.buffer.size, _vertexBuffer_F32PNTC.offset)
			}

			gpuBufferSize = _indexBuffer_F32PNTC.buffer.size;
			fullStagingBufferSize = indexArray_F32PNTC.size();
			if (fullStagingBufferSize >= gpuBufferSize)
			{
				_rhi->destroy_buffer(&_indexBuffer_F32PNTC.buffer);
				bufferInfo.size = fullStagingBufferSize * 2;
				bufferInfo.bufferUsage = rhi::ResourceUsage::INDEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
				_rhi->create_buffer(&_indexBuffer_F32PNTC.buffer, &bufferInfo);
				_rhi->copy_buffer(&cmdBuffer, &_stagingBuffers.indexBuffer_F32PNTC, &_indexBuffer_F32PNTC.buffer);
				_indexBuffer_F32PNTC.offset = _stagingBuffers.indexBuffer_F32PNTC.size;
				LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Reallocated index buffer F32PNTC. Size: {}. Offset: {}", _indexBuffer_F32PNTC.buffer.size, _indexBuffer_F32PNTC.offset)
			}
			else
			{
				gpuBufferOffset = _indexBuffer_F32PNTC.offset;
				_rhi->copy_buffer(
					&cmdBuffer,
					&_stagingBuffers.indexBuffer_F32PNTC,
					&_indexBuffer_F32PNTC.buffer,
					_stagingBuffers.indexBuffer_F32PNTC.size,
					0,
					gpuBufferOffset);
				_indexBuffer_F32PNTC.offset = _stagingBuffers.indexBuffer_F32PNTC.size;
				LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Added info about new models to index buffer F32PNTC. Size: {}. Offset: {}", _indexBuffer_F32PNTC.buffer.size, _indexBuffer_F32PNTC.offset)
			}
		}
	}
	else if (!_areGPUBuffersAllocated)
	{
		bufferInfo.size = DEFAULT_GPU_BUFFER_SIZE;
		bufferInfo.bufferUsage = rhi::ResourceUsage::VERTEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
		_rhi->create_buffer(&_vertexBuffer_F32PNTC.buffer, &bufferInfo);

		bufferInfo.bufferUsage = rhi::ResourceUsage::INDEX_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
		_rhi->create_buffer(&_indexBuffer_F32PNTC.buffer, &bufferInfo);

		_areGPUBuffersAllocated = true;
		
		LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Allocated default gpu buffers.")
		LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Vertex buffer F32PNTC. Size: {}. Offset: {}.", _vertexBuffer_F32PNTC.buffer.size, _vertexBuffer_F32PNTC.offset)
		LOG_INFO("ModelSubmanager::allocate_gpu_buffers(): Index buffer F32PNTC. Size: {}. Offset: {}.", _indexBuffer_F32PNTC.buffer.size, _indexBuffer_F32PNTC.offset)
	}
}

void ModelSubmanager::allocate_default_buffers(rhi::CommandBuffer& cmdBuffer)
{
	ecore::Plane plane;
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = plane.vertices.size() * sizeof(ecore::model::VertexF32PC);
	bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_SRC;
	bufferInfo.memoryUsage = rhi::MemoryUsage::CPU;
	_rhi->create_buffer(&_stagingBuffers.outputPlaneVertexBuffer_F32PC, &bufferInfo, plane.vertices.data());
	_stagingBuffersToDelete.push_back(&_stagingBuffers.outputPlaneVertexBuffer_F32PC);

	bufferInfo.bufferUsage = rhi::ResourceUsage::TRANSFER_DST | rhi::ResourceUsage::VERTEX_BUFFER;
	bufferInfo.memoryUsage = rhi::MemoryUsage::GPU;
	_rhi->create_buffer(&_outputPlaneVertexBuffer_F32PC, &bufferInfo);
	_rhi->copy_buffer(&cmdBuffer, &_stagingBuffers.outputPlaneVertexBuffer_F32PC, &_outputPlaneVertexBuffer_F32PC);
}
