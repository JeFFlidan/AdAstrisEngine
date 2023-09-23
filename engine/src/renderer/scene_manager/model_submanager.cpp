#include "model_submanager.h"
#include "engine_core/model/default_models.h"
#include "engine_core/engine_object_events.h"

using namespace ad_astris::renderer::impl;

ModelSubmanager::ModelSubmanager(SceneSubmanagerInitializationContext& initContext) : SceneSubmanagerBase(initContext)
{
	subscribe_to_events();
}

void ModelSubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	update_gpu_buffers(cmdBuffer);
}

void ModelSubmanager::cleanup_after_update()
{
	_loadedModelsVertexArraySize_F32PNTC = 0;
	_loadedModelsIndexArraySize_F32PNTC = 0;
	_loadedStaticModelHandles.clear();
}

bool ModelSubmanager::need_update()
{
	return !_areGPUBuffersAllocated || !_loadedStaticModelHandles.empty();
}

void ModelSubmanager::subscribe_to_events()
{
	events::EventDelegate<ecore::StaticModelCreatedEvent> staticModelLoadedDelegate = [&](ecore::StaticModelCreatedEvent& event)
	{
		UUID modelUUID = event.get_component_context()->modelComponent.modelUUID;
		_loadedStaticModelHandles.push_back(_resourceManager->get_resource<ecore::StaticModel>(modelUUID));

		ecore::StaticModelData modelData = _loadedStaticModelHandles.back().get_resource()->get_model_data();
		switch (_loadedStaticModelHandles.back().get_resource()->get_vertex_format())
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
	};

	_eventManager->subscribe(staticModelLoadedDelegate);
}

void ModelSubmanager::update_gpu_buffers(rhi::CommandBuffer& cmd)
{
	if (!_areGPUBuffersAllocated)
		allocate_gpu_buffers(cmd);
	update_cpu_arrays();
	if (_loadedModelsIndexArraySize_F32PNTC && _loadedModelsVertexArraySize_F32PNTC)
	{
		_rendererResourceManager->update_buffer(
			&cmd,
			VERTEX_BUFFER_F32PNTC_NAME,
			sizeof(uint8_t),
			_vertexArray_F32PNTC.data(),
			_vertexArray_F32PNTC.size(),
			_loadedModelsVertexArraySize_F32PNTC);
		_rendererResourceManager->update_buffer(
			&cmd,
			INDEX_BUFFER_F32PNTC_NAME,
			sizeof(uint8_t),
			_indexArray_F32PNTC.data(),
			_indexArray_F32PNTC.size(),
			_loadedModelsIndexArraySize_F32PNTC);
	}
}

void ModelSubmanager::allocate_gpu_buffers(rhi::CommandBuffer& cmd)
{
	_rendererResourceManager->allocate_storage_buffer(VERTEX_BUFFER_F32PNTC_NAME, DEFAULT_GPU_BUFFER_SIZE);
	_rendererResourceManager->allocate_storage_buffer(INDEX_BUFFER_F32PNTC_NAME, DEFAULT_GPU_BUFFER_SIZE);

	ecore::Plane plane;
	uint64_t size = plane.vertices.size() * sizeof(ecore::model::VertexF32PC);
	_rendererResourceManager->allocate_storage_buffer(OUTPUT_PLANE_VERTEX_BUFFER_NAME, size);
	_rendererResourceManager->update_buffer(
		&cmd,
		OUTPUT_PLANE_VERTEX_BUFFER_NAME,
		sizeof(ecore::model::VertexF32PC),
		plane.vertices.data(),
		plane.vertices.size(),
		plane.vertices.size());

	_areGPUBuffersAllocated = true;
}

void ModelSubmanager::update_cpu_arrays()
{
	uint32_t vertexArrayOffset_F32PNTC = _vertexArray_F32PNTC.size();
	uint32_t indexArrayOffset_F32PNTC = _indexArray_F32PNTC.size();
	_vertexArray_F32PNTC.resize(_vertexArray_F32PNTC.size() + _loadedModelsVertexArraySize_F32PNTC);
	_indexArray_F32PNTC.resize(_indexArray_F32PNTC.size() + _loadedModelsIndexArraySize_F32PNTC);

	ecore::StaticModelData staticModelData;
	for (auto& staticModelHandle : _loadedStaticModelHandles)
	{
		staticModelData = staticModelHandle.get_resource()->get_model_data();

		switch (staticModelHandle.get_resource()->get_vertex_format())
		{
			case ecore::model::VertexFormat::F32_PNTC:
			{
				memcpy(
					_vertexArray_F32PNTC.data() + vertexArrayOffset_F32PNTC,
					staticModelData.vertexBuffer,
					staticModelData.vertexBufferSize);
				memcpy(_indexArray_F32PNTC.data() + indexArrayOffset_F32PNTC,
					staticModelData.indexBuffer,
					staticModelData.indexBufferSize);
				break;
			}
			case ecore::model::VertexFormat::F32_PC:
			{
				//TODO
			}
		}
	}
}
