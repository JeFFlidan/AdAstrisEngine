#include "culling_submanager.h"
#include "common.h"
#include "engine_core/basic_events.h"

using namespace ad_astris;
using namespace renderer::impl;

constexpr uint32_t CULLING_PARAMS_COUNT = 128;

CullingSubmanager::CullingSubmanager()
{
	_cullingParamsCpuBuffer = std::make_unique<CpuBuffer<CullingParams>>(
		CPU_CULLING_PARAMS_BUFFER_NAME,
		sizeof(CullingParams) * CULLING_PARAMS_COUNT);
	_cullingParamsBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(
		CULLING_PARAMS_BUFFER_NAME,
		sizeof(CullingParams) * CULLING_PARAMS_COUNT);
	subscribe_to_events();
}

void CullingSubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	update_cpu_arrays();
	update_gpu_buffers(cmdBuffer);
}

void CullingSubmanager::cleanup_after_update()
{
	for (auto& pair : _sceneCullingContextByEntityFilterHash)
	{
		pair.second.cullingInstanceIndices->clear();
	}
}

bool CullingSubmanager::need_update()
{
	return true;
}

void CullingSubmanager::add_model(
	ecs::Entity entity,
	ecore::StaticModel* model,
	uint32_t vertexOffset,
	uint32_t indexOffset)
{
	SceneCullingContext* cullingContext = get_scene_culling_context(entity);
	auto it = cullingContext->indirectBatchIndexByModelUUID.find(model->get_uuid());
	if (it == cullingContext->indirectBatchIndexByModelUUID.end())
	{
		DrawIndexedIndirectCommand command;
		command.firstIndex = indexOffset;
		command.indexCount = model->get_model_data().indexBufferSize / sizeof(uint32_t);
		command.vertexOffset = vertexOffset;
		command.instanceCount = 0;		// will be filled in the culling compute shader
		command.firstInstance = cullingContext->instanceCount;
		uint32_t index = cullingContext->indirectCommands->get_element_count();
		cullingContext->indirectCommands->push_back(&command, 1);
		cullingContext->indirectBatchIndexByModelUUID[model->get_uuid()] = index;
	}
	
	++cullingContext->instanceCount;

	// TODO add model to shadows culling contexts
}

void CullingSubmanager::add_light(ecs::Entity entity)
{
	// TODO
}

void CullingSubmanager::add_culling_instance(ecs::Entity entity, uint32_t objectIndex, UUID modelUUID)
{
	SceneCullingContext* cpuIndirectData = get_scene_culling_context(entity);
	CullingInstanceIndices instanceIndices;
	// TODO Add some validation and logging
	instanceIndices.batchID = cpuIndirectData->indirectBatchIndexByModelUUID[modelUUID];
	instanceIndices.objectID = objectIndex;
	cpuIndirectData->cullingInstanceIndices->push_back(&instanceIndices, 1);
}

void CullingSubmanager::update_cpu_arrays()
{
	for (auto& pair : _sceneCullingContextByEntityFilterHash)
	{
		SceneCullingContext& cullingContext = pair.second;
		for (auto& pair2 : cullingContext.indirectBuffersByCameraIndex)
		{
			const ecs::Entity camera = _cameras[pair2.first];
			const ecore::CameraComponent* cameraComponent = camera.get_component<ecore::CameraComponent>();
			
			const ecore::SceneCullingSettings& sceneCullingSettings = RENDERER_SUBSETTINGS()->get_scene_culling_settings();
			CullingParams* cullingParams = _cullingParamsCpuBuffer->get_data(pair2.second.cullingParamsIndex);
			cullingParams->cameraIndex = pair2.first;
 			cullingParams->drawCount = cullingContext.instanceCount;
			cullingParams->isFrustumCullingEnabled = sceneCullingSettings.isFrustumCullingEnabled;
			cullingParams->isOcclusionCullingEnabled = sceneCullingSettings.isOcclusionCullingEnabled;
			cullingParams->isAABBCheckEnabled = false;
			// TODO write whole cullingParams configuration
		}
	}
}

void CullingSubmanager::update_gpu_buffers(rhi::CommandBuffer& cmd)
{
	for (auto& pair : _sceneCullingContextByEntityFilterHash)
	{
		SceneCullingContext& cullingContext = pair.second;
		if (!cullingContext.indirectCommands->empty() && !cullingContext.cullingInstanceIndices->empty())
		{
			for (auto& pair2 : cullingContext.indirectBuffersByCameraIndex)
			{
				IndirectBuffers& indirectBuffers = pair2.second;
				RENDERER_RESOURCE_MANAGER()->update_buffer(
				   &cmd,
				   cullingContext.indirectCommands->get_buffer(),
				   indirectBuffers.indirectBuffer,
				   sizeof(DrawIndexedIndirectCommand),
				   cullingContext.indirectCommands->get_element_count(),
				   cullingContext.indirectCommands->get_element_count());
				RENDERER_RESOURCE_MANAGER()->update_buffer(
					&cmd,
					cullingContext.cullingInstanceIndices->get_buffer(),
					cullingContext.cullingInstanceIndicesBuffer,
					sizeof(CullingInstanceIndices),
					cullingContext.cullingInstanceIndices->get_element_count(),
					cullingContext.cullingInstanceIndices->get_element_count());
				RENDERER_RESOURCE_MANAGER()->update_buffer(
					&cmd,
					_cullingParamsCpuBuffer->get_buffer(),
					_cullingParamsBuffer,
					sizeof(CullingParams),
					_cullingParamsCpuBuffer->get_element_count(),
					_cullingParamsCpuBuffer->get_element_count());
				// TODO update lights buffers
			}
		}
	}
}

CullingSubmanager::SceneCullingContext* CullingSubmanager::get_scene_culling_context(ecs::Entity entity)
{
	SceneCullingContext* cpuIndirectData = nullptr;
	
	for (auto& filter : _sceneEntityFilters)
	{
		if (filter->validate(entity))
		{
			cpuIndirectData = &_sceneCullingContextByEntityFilterHash.find(filter->get_requirements_hash())->second;
		}
	}

	if (!cpuIndirectData)
	{
		LOG_FATAL("IndirectDrawingSubmanager::add_model(): Entity {} does not match any EntityFilter", entity)
	}

	return cpuIndirectData;
}

void CullingSubmanager::subscribe_to_events()
{
	events::EventDelegate<ecore::CameraSetEvent> delegate1 = [this](ecore::CameraSetEvent& event)
	{
		_cameras[event.get_camera_index()] = event.get_camera();
	};
	EVENT_MANAGER()->subscribe(delegate1);
}
