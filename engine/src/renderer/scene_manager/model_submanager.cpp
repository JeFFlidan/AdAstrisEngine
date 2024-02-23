#include "model_submanager.h"
#include "engine_core/model/default_models.h"
#include "engine_core/engine_object_events.h"
#include "engine_core/basic_events.h"
#include "core/global_objects.h"

using namespace ad_astris;
using namespace renderer::impl;

ModelSubmanager::ModelSubmanager(MaterialSubmanager* materialSubmanager, CullingSubmanager* indirectDrawingSubmanager)
	: _materialSubmanager(materialSubmanager), _indirectDrawingSubmanager(indirectDrawingSubmanager)
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
	_createdModels.clear();
	_modelInstances.clear();
}

bool ModelSubmanager::need_update()
{
	return !_areGPUBuffersAllocated || ! _createdModels.empty();
}

void ModelSubmanager::subscribe_to_events()
{
	
}

void ModelSubmanager::update_gpu_buffers(rhi::CommandBuffer& cmd)
{
	if (!_areGPUBuffersAllocated)
		allocate_gpu_buffers(cmd);

	update_cpu_arrays(cmd);

	if (!_modelInstances.empty())
	{
		RENDERER_RESOURCE_MANAGER()->update_buffer(
			&cmd,
			MODEL_INSTANCE_BUFFER_NAME,
			sizeof(RendererModelInstance),
			_modelInstances.data(),
			_modelInstances.size(),
			_modelInstances.size());
	}
	
	if (_loadedModelsIndexArraySize_F32PNTC && _loadedModelsVertexArraySize_F32PNTC)
	{
		RENDERER_RESOURCE_MANAGER()->update_buffer(
			&cmd,
			VERTEX_BUFFER_F32PNTC_NAME,
			sizeof(uint8_t),
			_vertexArray_F32PNTC.data(),
			_vertexArray_F32PNTC.size(),
			_loadedModelsVertexArraySize_F32PNTC);
		RENDERER_RESOURCE_MANAGER()->update_buffer(
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
	RENDERER_RESOURCE_MANAGER()->allocate_vertex_buffer(VERTEX_BUFFER_F32PNTC_NAME, DEFAULT_GPU_BUFFER_SIZE);
	RENDERER_RESOURCE_MANAGER()->allocate_index_buffer(INDEX_BUFFER_F32PNTC_NAME, DEFAULT_GPU_BUFFER_SIZE);
	RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(MODEL_INSTANCE_BUFFER_NAME, MODEL_INSTANCES_INIT_NUMBER * sizeof(RendererModelInstance));

	ecore::Plane plane;
	uint64_t size = plane.vertices.size() * sizeof(ecore::model::VertexF32PC);
	rhi::Buffer* buffer = RENDERER_RESOURCE_MANAGER()->allocate_vertex_buffer(OUTPUT_PLANE_VERTEX_BUFFER_NAME, size);
	RENDERER_RESOURCE_MANAGER()->update_buffer(
		&cmd,
		OUTPUT_PLANE_VERTEX_BUFFER_NAME,
		sizeof(ecore::model::VertexF32PC),
		plane.vertices.data(),
		plane.vertices.size(),
		plane.vertices.size());
	_areGPUBuffersAllocated = true;
}

void ModelSubmanager::update_cpu_arrays(rhi::CommandBuffer& cmd)
{
	uint32_t vertexArrayOffset_F32PNTC = _vertexArray_F32PNTC.size();
	uint32_t indexArrayOffset_F32PNTC = _indexArray_F32PNTC.size();
	
	for (auto entity : _createdModels)
	{
		if (entity.has_tag<ecore::StaticObjectTag>())
		{
			ecore::StaticModel* model = get_static_model_handle(entity).get_resource();
			ecore::StaticModelData modelData = model->get_model_data();
			_indirectDrawingSubmanager->add_model(entity, model, vertexArrayOffset_F32PNTC, indexArrayOffset_F32PNTC);
			if (!is_model_loaded(model->get_uuid()))
			{
				switch (model->get_vertex_format())
				{
					case ecore::model::VertexFormat::F32_PNTC:
					{
						_vertexArray_F32PNTC.resize(_vertexArray_F32PNTC.size() + modelData.vertexBufferSize);
						_indexArray_F32PNTC.resize(_indexArray_F32PNTC.size() + modelData.indexBufferSize);
						memcpy(
							_vertexArray_F32PNTC.data() + vertexArrayOffset_F32PNTC,
							modelData.vertexBuffer,
							modelData.vertexBufferSize);
						memcpy(_indexArray_F32PNTC.data() + indexArrayOffset_F32PNTC,
							modelData.indexBuffer,
							modelData.indexBufferSize);
						vertexArrayOffset_F32PNTC += modelData.vertexBufferSize;
						indexArrayOffset_F32PNTC += modelData.indexBufferSize;
						_loadedModelsVertexArraySize_F32PNTC += modelData.vertexBufferSize;
						_loadedModelsIndexArraySize_F32PNTC += modelData.indexBufferSize;
						break;
					}
					case ecore::model::VertexFormat::F32_PC:
					{
						//TODO
					}
				}
			}

			_entitiesByModelUUID[model->get_uuid()].push_back(entity);
		}
	}

	for (const auto& pair : _entitiesByModelUUID)
	{
		for (const auto& entity : pair.second)
		{
			auto modelComponent = entity.cget_component<ecore::ModelComponent>();
			ecore::StaticModelHandle modelHandle = RESOURCE_MANAGER()->get_resource<ecore::StaticModel>(modelComponent->modelUUID);
			ecore::StaticModel* staticModel = modelHandle.get_resource();

			uint32_t modelInstanceIndex = _modelInstances.size();
			_indirectDrawingSubmanager->add_culling_instance(entity, modelInstanceIndex, staticModel->get_uuid());
		
			RendererModelInstance& modelInstance = _modelInstances.emplace_back();
			modelInstance.materialIndex = 0;	// TODO change
			ecore::model::ModelBounds bounds = staticModel->get_model_bounds();
			modelInstance.sphereBoundsRadius = bounds.radius;
			modelInstance.sphereBoundsCenter = bounds.origin;
			auto transform = entity.cget_component<ecore::TransformComponent>();
			modelInstance.transform.set_transfrom(transform->world);

			XMMATRIX matrix = XMLoadFloat4x4(&transform->world);
			matrix = XMMatrixInverse(nullptr, XMMatrixTranspose(matrix));
			XMFLOAT4X4 transformInverseTranspose;
			XMStoreFloat4x4(&transformInverseTranspose, matrix);
			modelInstance.transformInverseTranspose.set_transfrom(transformInverseTranspose);		// TODO change matrix

			if (entity.has_component<ecore::OpaquePBRMaterialComponent>())
			{
				auto materialComponent = entity.cget_component<ecore::OpaquePBRMaterialComponent>();
				modelInstance.materialIndex = _materialSubmanager->get_gpu_material_index(cmd, materialComponent->materialUUID);
			}
			// TODO transparent materials
		}
	}
}

void ModelSubmanager::add_model(ecs::Entity entity)
{
	_staticModelEntities.insert(entity);
	auto modelHandle = get_static_model_handle(entity);
	ecore::StaticModelData modelData = modelHandle.get_resource()->get_model_data();
	_createdModels.push_back(entity);
}

resource::ResourceAccessor<ecore::StaticModel> ModelSubmanager::get_static_model_handle(ecs::Entity entity)
{
	const ecore::ModelComponent* modelComponent = entity.get_component<ecore::ModelComponent>();
	return RESOURCE_MANAGER()->get_resource<ecore::StaticModel>(modelComponent->modelUUID);
}

bool ModelSubmanager::is_model_loaded(UUID modelUUID)
{
	return _entitiesByModelUUID.find(modelUUID) != _entitiesByModelUUID.end();
}
