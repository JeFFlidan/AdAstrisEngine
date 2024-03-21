#include "culling_submanager.h"
#include "common.h"
#include "engine_core/basic_events.h"

using namespace ad_astris;
using namespace renderer::impl;

constexpr uint32_t CULLING_PARAMS_COUNT = 128;

CullingSubmanager::CullingSubmanager()
{
	_cullingParamsCpuBuffer = std::make_unique<RendererArray<CullingParams>>(
		CPU_CULLING_PARAMS_BUFFER_NAME,
		sizeof(CullingParams) * CULLING_PARAMS_COUNT);
	_cullingParamsBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(
		CULLING_PARAMS_BUFFER_NAME,
		sizeof(CullingParams) * CULLING_PARAMS_COUNT);
	subscribe_to_events();
}

void CullingSubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	update_cpu_arrays(cmdBuffer);
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
	auto it = cullingContext->indirectBatchMiscByModelUUID.find(model->get_uuid());
	if (it == cullingContext->indirectBatchMiscByModelUUID.end())
	{
		DrawIndexedIndirectCommand command;
		command.firstIndex = indexOffset;
		command.indexCount = model->get_model_data().indexBufferSize / sizeof(uint32_t);
		command.vertexOffset = vertexOffset;
		command.instanceCount = 0;		// will be filled in the culling compute shader
		command.firstInstance = cullingContext->instanceCount;
		uint32_t index = cullingContext->indirectCommands->get_element_count();
		cullingContext->indirectCommands->push_back(&command, 1);
		cullingContext->indirectBatchMiscByModelUUID[model->get_uuid()] = { index, 0 };
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
	instanceIndices.batchID = cpuIndirectData->indirectBatchMiscByModelUUID[modelUUID].index;
	instanceIndices.objectID = objectIndex;
	++cpuIndirectData->indirectBatchMiscByModelUUID[modelUUID].instanceCount;
	cpuIndirectData->cullingInstanceIndices->push_back(&instanceIndices, 1);
}

void CullingSubmanager::update_cpu_arrays(rhi::CommandBuffer& cmd)
{
	for (auto& pair : _sceneCullingContextByEntityFilterHash)
	{
		SceneCullingContext& cullingContext = pair.second;

		uint32_t instanceOffset = 0;
		for (auto& pair2 : cullingContext.indirectBatchMiscByModelUUID)
		{
			DrawIndexedIndirectCommand* indirectCommand = cullingContext.indirectCommands->get_data(pair2.second.index);
			indirectCommand->firstInstance = instanceOffset;
			instanceOffset += pair2.second.instanceCount;
			pair2.second.instanceCount = 0;
		}
		
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
			IndirectBuffers& indirectBuffers = pair2.second;
			cullingParams->depthPyramidIndex = RHI()->get_descriptor_index(indirectBuffers.depthPyramid->get_texture_view());
			cullingParams->pyramidWidth = indirectBuffers.depthPyramid->get_width();
			cullingParams->pyramidHeight = indirectBuffers.depthPyramid->get_height();
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
					cullingContext.cullingInstanceIndices->get_mapped_buffer(),
					cullingContext.cullingInstanceIndices->get_gpu_buffer(),
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

DepthPyramid::DepthPyramid(size_t entityFilterHash) : _entityFilterHash(entityFilterHash)
{
	_width = math::previous_pow2(IMAGE_WIDTH);
	_height = math::previous_pow2(IMAGE_HEIGHT);
	_mipLevels = math::get_mip_levels(_width, _height);
	RENDERER_RESOURCE_MANAGER()->allocate_gpu_texture(
		get_str_with_id(DEPTH_PYRAMID_NAME),
		_width,
		_height,
		rhi::Format::R32_SFLOAT,
		rhi::ResourceUsage::STORAGE_TEXTURE | rhi::ResourceUsage::TRANSFER_SRC | rhi::ResourceUsage::SAMPLED_TEXTURE,
		rhi::ResourceFlags::UNDEFINED,
		_mipLevels);
	
	RENDERER_RESOURCE_MANAGER()->allocate_texture_view(
		get_str_with_id(DEPTH_PYRAMID_NAME),
		get_str_with_id(DEPTH_PYRAMID_NAME));

	for (uint32_t i = 0; i != _mipLevels; ++i)
	{
		RENDERER_RESOURCE_MANAGER()->allocate_texture_view(
			get_str_with_id(DEPTH_PYRAMID_MIPMAP_NAME) + std::to_string(i),
			get_str_with_id(DEPTH_PYRAMID_NAME),
			i, 1);
	}
}
