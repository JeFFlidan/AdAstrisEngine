#include "model_submanager.h"
#include "engine_core/model/default_models.h"
#include "engine_core/engine_object_events.h"
#include "engine_core/basic_events.h"
#include "core/global_objects.h"

using namespace ad_astris::renderer::impl;

constexpr uint32_t MODEL_INSTANCES_INIT_NUMBER = 512;

ModelSubmanager::ModelSubmanager(MaterialSubmanager* materialSubmanager)
	: _materialSubmanager(materialSubmanager)
{
	subscribe_to_events();
	_modelInstances.reserve(MODEL_INSTANCES_INIT_NUMBER);
	_staticModelEntities.reserve(MODEL_INSTANCES_INIT_NUMBER);
	_indirectBufferDesc = std::make_unique<IndirectBufferDesc>(this, 0);	// TEMP
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
	_indirectBufferDesc->cleanup_after_update();
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
	
	_indirectBufferDesc->update_gpu_buffers(cmd);
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
	_vertexArray_F32PNTC.resize(_vertexArray_F32PNTC.size() + _loadedModelsVertexArraySize_F32PNTC);
	_indexArray_F32PNTC.resize(_indexArray_F32PNTC.size() + _loadedModelsIndexArraySize_F32PNTC);

	ecore::StaticModelData staticModelData;
	for (auto& staticModelHandle : _loadedStaticModelHandles)
	{
		staticModelData = staticModelHandle.get_resource()->get_model_data();

		if (_indirectBufferDesc->add_model(staticModelHandle.get_resource()->get_uuid(), vertexArrayOffset_F32PNTC, indexArrayOffset_F32PNTC, staticModelData))
		{
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
					vertexArrayOffset_F32PNTC += staticModelData.vertexBufferSize;
					indexArrayOffset_F32PNTC += staticModelData.indexBufferSize;
					break;
				}
				case ecore::model::VertexFormat::F32_PC:
				{
					//TODO
				}
			}
		}
	}

	ecs::EntityManager* entityManager = WORLD()->get_entity_manager();
	uint32_t instanceCounter = 0;
	for (auto uuid : _indirectBufferDesc->get_uuids())
	{
		for (auto& entity : _entitiesByModelUUID[uuid])
		{
			auto modelComponent = entityManager->get_component_const<ecore::ModelComponent>(entity);
			ecore::StaticModelHandle modelHandle = RESOURCE_MANAGER()->get_resource<ecore::StaticModel>(modelComponent->modelUUID);
			ecore::StaticModel* staticModel = modelHandle.get_resource();

			uint32_t modelInstanceIndex = _modelInstances.size();
			_indirectBufferDesc->add_instance(modelInstanceIndex, staticModel->get_uuid());
		
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
				modelInstance.materialIndex = _materialSubmanager->get_gpu_material_index(cmd, materialComponent->materialUUID);
			}
			// TODO transparent materials

			_indirectBufferDesc->add_renderer_model_instance_id(instanceCounter++);
		}
	}
}

void ModelSubmanager::add_static_model(ecore::StaticModelHandle handle, ecs::Entity& entity)
{
	_staticModelEntities.insert(entity);
	_loadedStaticModelHandles.push_back(handle);
	ecore::StaticModelData modelData = handle.get_resource()->get_model_data();
	_loadedModelsVertexArraySize_F32PNTC += modelData.vertexBufferSize;
	_loadedModelsIndexArraySize_F32PNTC += modelData.indexBufferSize;
	_entitiesByModelUUID[handle.get_resource()->get_uuid()].push_back(entity);
}

IndirectBufferDesc::IndirectBufferDesc(ModelSubmanager* modelSubmanager, uint32_t indirectBufferIndex)
	: _modelSubmanager(modelSubmanager), _indirectBufferIndex(indirectBufferIndex)
{
	allocate_gpu_buffers();
}

void IndirectBufferDesc::cleanup_after_update()
{
	_cullingInstanceIndices.clear();
	_modelInstanceIDs.clear();
}

bool IndirectBufferDesc::add_model(
	UUID cpuModelUUID,
	uint32_t vertexOffset,
	uint32_t indexOffset,
	ecore::StaticModelData& modelData)
{
	auto it = _indirectBatchIndexByCPUModelUUID.find(cpuModelUUID);
	if (it == _indirectBatchIndexByCPUModelUUID.end())
	{
		uint32_t index = _indirectCommands.size();
		DrawIndexedIndirectCommand& command = _indirectCommands.emplace_back();
		command.firstIndex = indexOffset;
		command.indexCount = modelData.indexBufferSize / sizeof(uint32_t);
		command.vertexOffset = vertexOffset;
		command.instanceCount = 1;		// will be filled in the culling compute shader
		command.firstInstance = _instanceCount++;
		_indirectBatchIndexByCPUModelUUID[cpuModelUUID] = index;
		_uuidInRightOrder.push_back(cpuModelUUID);
		return true;
	}
	
	_indirectCommands[it->second].instanceCount += 1;
	++_instanceCount;
	return false;
}

void IndirectBufferDesc::add_instance(uint32_t objectIndex, UUID modelUUID)
{
	CullingInstanceIndices& instanceIndices = _cullingInstanceIndices.emplace_back();
	instanceIndices.batchID = _indirectBatchIndexByCPUModelUUID[modelUUID];
	instanceIndices.instanceID = objectIndex;
}

void IndirectBufferDesc::update_gpu_buffers(rhi::CommandBuffer& cmd)
{
	if (!_indirectCommands.empty() && !_modelInstanceIDs.empty())
	{
		RENDERER_RESOURCE_MANAGER()->update_buffer(
			&cmd,
			get_name_with_index("indirect_buffer"),
			sizeof(DrawIndexedIndirectCommand),
			_indirectCommands.data(),
			_indirectCommands.size(),
			_indirectCommands.size());

		// _modelSubmanager->get_renderer_resource_manager()->update_buffer(
		// 	&cmd,
		// 	get_name_with_index("culling_indices_buffer"),
		// 	sizeof(CullingInstanceIndices),
		// 	_cullingInstanceIndices.data(),
		// 	_cullingInstanceIndices.size(),
		// 	_cullingInstanceIndices.size());

		RENDERER_RESOURCE_MANAGER()->update_buffer(
			&cmd,
			get_name_with_index("model_instance_id_buffer"),
			sizeof(RendererModelInstanceID),
			_modelInstanceIDs.data(),
			_modelInstanceIDs.size(),
			_modelInstanceIDs.size());
	}
}

void IndirectBufferDesc::allocate_gpu_buffers()
{
	_indirectBuffer = RENDERER_RESOURCE_MANAGER()->allocate_indirect_buffer(
		get_name_with_index("indirect_buffer"),
		sizeof(DrawIndexedIndirectCommand) * MODEL_INSTANCES_INIT_NUMBER);
	// _modelSubmanager->get_renderer_resource_manager()->allocate_storage_buffer(
	// 	get_name_with_index("culling_indices_buffer"),
	// 	sizeof(CullingInstanceIndices) * MODEL_INSTANCES_INIT_NUMBER);
	_modelInstanceIDBuffer = RENDERER_RESOURCE_MANAGER()->allocate_storage_buffer(
		get_name_with_index("model_instance_id_buffer"),
		sizeof(RendererModelInstanceID) * MODEL_INSTANCES_INIT_NUMBER);
}

std::string IndirectBufferDesc::get_name_with_index(const std::string& name)
{
	return name + std::to_string(_indirectBufferIndex);
}
