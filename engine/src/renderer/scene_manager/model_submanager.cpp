#include "model_submanager.h"
#include "engine_core/model/default_models.h"
#include "engine_core/engine_object_events.h"
#include "engine_core/basic_events.h"

using namespace ad_astris::renderer::impl;

constexpr uint32_t MODEL_INSTANCES_INIT_NUMBER = 512;

ModelSubmanager::ModelSubmanager(SceneSubmanagerInitializationContext& initContext, MaterialSubmanager* materialSubmanager)
	: SceneSubmanagerBase(initContext), _materialSubmanager(materialSubmanager)
{
	subscribe_to_events();
	_modelInstances.reserve(MODEL_INSTANCES_INIT_NUMBER);
	_staticModelEntities.reserve(MODEL_INSTANCES_INIT_NUMBER);
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
	_modelInstances.clear();
}

bool ModelSubmanager::need_update()
{
	return !_areGPUBuffersAllocated || !_loadedStaticModelHandles.empty();
}

void ModelSubmanager::subscribe_to_events()
{
	
}

void ModelSubmanager::update_gpu_buffers(rhi::CommandBuffer& cmd)
{
	if (!_areGPUBuffersAllocated)
		allocate_gpu_buffers(cmd);

	update_cpu_arrays();

	if (!_modelInstances.empty())
	{
		_rendererResourceManager->update_buffer(
			&cmd,
			MODEL_INSTANCE_BUFFER_NAME,
			sizeof(RendererModelInstance),
			_modelInstances.data(),
			_modelInstances.size(),
			_modelInstances.size());
	}
	
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
	_rendererResourceManager->allocate_vertex_buffer(VERTEX_BUFFER_F32PNTC_NAME, DEFAULT_GPU_BUFFER_SIZE);
	_rendererResourceManager->allocate_index_buffer(INDEX_BUFFER_F32PNTC_NAME, DEFAULT_GPU_BUFFER_SIZE);
	_rendererResourceManager->allocate_storage_buffer(MODEL_INSTANCE_BUFFER_NAME, MODEL_INSTANCES_INIT_NUMBER * sizeof(RendererModelInstance));

	ecore::Plane plane;
	uint64_t size = plane.vertices.size() * sizeof(ecore::model::VertexF32PC);
	rhi::Buffer* buffer = _rendererResourceManager->allocate_vertex_buffer(OUTPUT_PLANE_VERTEX_BUFFER_NAME, size);
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

	ecs::EntityManager* entityManager = _world->get_entity_manager();
	for (auto entity : _staticModelEntities)
	{
		auto modelComponent = entityManager->get_component_const<ecore::ModelComponent>(entity);
		ecore::StaticModelHandle modelHandle = _resourceManager->get_resource<ecore::StaticModel>(modelComponent->modelUUID);
		ecore::StaticModel* staticModel = modelHandle.get_resource();
		RendererModelInstance& modelInstance = _modelInstances.emplace_back();
		modelInstance.materialIndex = 0;	// TODO change
		ecore::model::ModelBounds bounds = staticModel->get_model_bounds();
		modelInstance.sphereBoundsRadius = bounds.radius;
		modelInstance.sphereBoundsCenter = bounds.origin;
		auto transform = entityManager->get_component_const<ecore::TransformComponent>(entity);
		modelInstance.transform.set_transfrom(transform->world);

		XMMATRIX matrix = XMLoadFloat4x4(&transform->world);
		matrix = XMMatrixInverse(nullptr, XMMatrixTranspose(matrix));
		XMFLOAT4X4 transformInverseTranspose;
		XMStoreFloat4x4(&transformInverseTranspose, matrix);
		modelInstance.transformInverseTranspose.set_transfrom(transformInverseTranspose);		// TODO change matrix

		if (entityManager->does_entity_have_component<ecore::OpaquePBRMaterialComponent>(entity))
		{
			auto materialComponent = entityManager->get_component<ecore::OpaquePBRMaterialComponent>(entity);
			modelInstance.materialIndex = _materialSubmanager->get_gpu_material_index(materialComponent->materialUUID);
		}
		// TODO transparent materials
	}
}

void ModelSubmanager::add_static_model(ecore::StaticModelHandle handle, ecs::Entity& entity)
{
	_staticModelEntities.insert(entity);
	_loadedStaticModelHandles.push_back(handle);
	ecore::StaticModelData modelData = handle.get_resource()->get_model_data();
	_loadedModelsVertexArraySize_F32PNTC += modelData.vertexBufferSize;
	_loadedModelsIndexArraySize_F32PNTC += modelData.indexBufferSize;
}
