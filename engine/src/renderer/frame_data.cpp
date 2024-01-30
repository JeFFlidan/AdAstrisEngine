#include "frame_data.h"
#include "scene_manager/scene_manager.h"
#include "module_objects.h"
#include "engine_core/basic_components.h"

using namespace ad_astris;
using namespace renderer::impl;

const std::string CAMERA_UB_NAME = "CameraUB";
const std::string FRAME_UB_NAME = "FrameUB";

void FrameData::init()
{
	rhi::BufferInfo bufferInfo;
	bufferInfo.size = sizeof(RendererCamera) * MAX_CAMERA_COUNT;
	bufferInfo.bufferUsage = rhi::ResourceUsage::UNIFORM_BUFFER | rhi::ResourceUsage::TRANSFER_DST;
	bufferInfo.memoryUsage = rhi::MemoryUsage::CPU_TO_GPU;
	for (uint32_t i = 0; i != RHI()->get_buffer_count(); ++i)
		RENDERER_RESOURCE_MANAGER()->allocate_buffer(get_buffer_name(CAMERA_UB_NAME, i), bufferInfo);

	bufferInfo.size = sizeof(FrameUB);
	for (uint32_t i = 0; i != RHI()->get_buffer_count(); ++i)
		RENDERER_RESOURCE_MANAGER()->allocate_buffer(get_buffer_name(FRAME_UB_NAME, i), bufferInfo);
}

void FrameData::update_uniform_buffers(DrawContext& drawContext, uint32_t frameIndex)
{
	_frameIndex = frameIndex;
	setup_cameras(drawContext);
	setup_frame_data(drawContext);
}

void FrameData::setup_cameras(DrawContext& drawContext)
{
	auto cameraComponent = WORLD()->get_entity_manager()->get_component<ecore::CameraComponent>(drawContext.activeCamera);
	auto transformComponent = WORLD()->get_entity_manager()->get_component_const<ecore::TransformComponent>(drawContext.activeCamera);
	_cameras[0].location = cameraComponent->eye;
	_cameras[0].view = cameraComponent->view;
	_cameras[0].projection = cameraComponent->projection;
	_cameras[0].viewProjection = cameraComponent->viewProjection;
	_cameras[0].inverseView = cameraComponent->inverseView;
	_cameras[0].inverseProjection = cameraComponent->inverseProjection;
	_cameras[0].inverseViewProjection = cameraComponent->inverseViewProjection;
	_cameras[0].zNear = cameraComponent->zNear;
	_cameras[0].zFar = cameraComponent->zFar;
	_cameras[0].up = cameraComponent->up;
	rhi::Buffer* cameraUB = RENDERER_RESOURCE_MANAGER()->get_buffer(get_buffer_name(CAMERA_UB_NAME));
	RHI()->update_buffer_data(cameraUB, sizeof(RendererCamera) * MAX_CAMERA_COUNT, _cameras.data());
	RHI()->bind_uniform_buffer(cameraUB, UB_CAMERA_SLOT);
}

void FrameData::setup_frame_data(DrawContext& drawContext)
{
	rhi::Buffer* modelInstanceBuffer = SCENE_MANAGER()->get_model_instance_storage_buffer();
	rhi::Buffer* materialBuffer = SCENE_MANAGER()->get_material_storage_buffer();
	rhi::Buffer* entityBuffer = SCENE_MANAGER()->get_entity_storage_buffer();
	rhi::Buffer* modelInstanceIDBuffer = SCENE_MANAGER()->get_indirect_buffer_desc()->get_model_instance_id_buffer();
	_frameData.modelInstanceBufferIndex = RHI()->get_descriptor_index(modelInstanceBuffer);
	_frameData.materialBufferIndex = RHI()->get_descriptor_index(materialBuffer);
	_frameData.entityBufferIndex = RHI()->get_descriptor_index(entityBuffer);
	_frameData.modelInstanceIDBufferIndex = RHI()->get_descriptor_index(modelInstanceIDBuffer);
	_frameData.lightArrayOffset = 0;
	_frameData.lightArrayCount = SCENE_MANAGER()->get_light_count();	// TODO Take info from scene manager
	rhi::Buffer* frameUB = RENDERER_RESOURCE_MANAGER()->get_buffer(get_buffer_name(FRAME_UB_NAME));
	RHI()->update_buffer_data(frameUB, sizeof(FrameUB), &_frameData);
	RHI()->bind_uniform_buffer(frameUB, UB_FRAME_SLOT);
}
