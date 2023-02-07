#include "vk_scene.h"
#include "material_system.h"

#include "mesh_asset.h"
#include "vk_engine.h"
#include "vk_initializers.h"

#include <atomic>
#include <iostream>
#include <future>
#include <algorithm>
#include <iterator>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

//public methods

void RenderScene::init()
{
	_forwardPass.type = vkutil::MeshpassType::Forward;
	_dirShadowPass.type = vkutil::MeshpassType::DirectionalShadow;
	_transparentForwardPass.type = vkutil::MeshpassType::Transparency;
	_pointShadowPass.type = vkutil::MeshpassType::PointShadow;
	_spotShadowPass.type = vkutil::MeshpassType::SpotShadow;
	_deferredPass.type = vkutil::MeshpassType::Deferred;
}

void RenderScene::cleanup(VulkanEngine* engine)
{
	std::vector<MeshPass*> passes = { &_dirShadowPass, &_pointShadowPass, &_spotShadowPass, &_transparentForwardPass, &_deferredPass };
	for (auto pass : passes)
	{
		pass->compactedInstanceBuffer.destroy_buffer(engine);
		pass->passObjectsBuffer.destroy_buffer(engine);
		pass->drawIndirectBuffer.destroy_buffer(engine);
		pass->clearIndirectBuffer.destroy_buffer(engine);
	}

	_pointLightsBuffer.destroy_buffer(engine);
	_spotLightsBuffer.destroy_buffer(engine);
	_dirLightsBuffer.destroy_buffer(engine);
	_objectDataBuffer.destroy_buffer(engine);

	for (auto& shadowMap : _dirShadowMaps)
	{
		shadowMap.lightBuffer.destroy_buffer(engine);
	}
	for (auto& shadowMap : _pointShadowMaps)
	{
		shadowMap.lightBuffer.destroy_buffer(engine);
	}
	for (auto& shadowMap : _spotShadowMaps)
	{
		shadowMap.lightBuffer.destroy_buffer(engine);
	}
}

void RenderScene::register_object_batch(MeshObject* first, uint32_t count)
{
	_renderables.reserve(count);

	for (int i = 0; i != count; ++i)
	{
		register_object(&(first[i]));
	}
}

Handle<RenderableObject> RenderScene::register_object(MeshObject* object)
{
	RenderableObject newObj;
	newObj.transformMatrix = object->transformMatrix;
	newObj.meshID = get_mesh_handle(object->mesh);
	newObj.material = get_material_handle(object->material);
	newObj.updateIndex = (uint32_t)-1;		// Why in guide is (uint_32) used?
	newObj.customSortKey = object->customSortKey;
	newObj.passIndeces.clear(-1);
	Handle<RenderableObject> handle;
	handle.handle = static_cast<uint32_t>(_renderables.size());
	
	//_renderables.push_back(newObj);

	if (object->bDrawForwardPass)
	{
		if (object->material->original->passShaders[vkutil::MeshpassType::Forward])
		{
			_forwardPass.unbatchedObjects.push_back(handle);
		}
	}

	if (object->bDrawTransparencyPass)
	{
		if (object->material->original->passShaders[vkutil::MeshpassType::Transparency])
		{
			LOG_INFO("+1 in transparency pass");
			_transparentForwardPass.unbatchedObjects.push_back(handle);
		}
	}

	if (object->bDrawShadowPass)
	{
		if (object->material->original->passShaders[vkutil::MeshpassType::DirectionalShadow])
		{
			LOG_INFO("+1 in directional shadow pass");
			_dirShadowPass.unbatchedObjects.push_back(handle);
		}
	}

	if (object->bDrawPointShadowPass)
	{
		if (object->material->original->passShaders[vkutil::MeshpassType::PointShadow])
		{
			LOG_INFO("+1 in point shadow pass");
			_pointShadowPass.unbatchedObjects.push_back(handle);
		}
	}

	if (object->bDrawSpotShadowPass)
	{
		if (object->material->original->passShaders[vkutil::MeshpassType::SpotShadow])
		{
			LOG_INFO("+1 in spot shadow pass");
			_spotShadowPass.unbatchedObjects.push_back(handle);
		}
	}

	if (object->bDrawDeferredPass)
	{
		if (object->material->original->passShaders[vkutil::MeshpassType::Deferred])
		{
			LOG_INFO("+1 in deferred pass");
			_deferredPass.unbatchedObjects.push_back(handle);
		}
	}

	if (object->baseColor != VK_NULL_HANDLE)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = object->baseColor;
		imageInfo.sampler = nullptr;

		newObj.baseColorTexId = _baseColorInfos.size();

		_baseColorInfos.push_back(imageInfo);
	}
	
	if (object->normal != VK_NULL_HANDLE)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = object->normal;
		imageInfo.sampler = nullptr;

		newObj.normalTexId = _normalInfos.size();

		_normalInfos.push_back(imageInfo);
	}
	
	if (object->arm != VK_NULL_HANDLE)
	{
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = object->arm;
		imageInfo.sampler = nullptr;

		newObj.armTexId = _armInfos.size();
	
		_armInfos.push_back(imageInfo);
	}

	_renderables.push_back(newObj);

	update_object(handle);
	return handle;
}

void RenderScene::update_object(Handle<RenderableObject> objectID)
{
	auto& passIndices = get_renderable_object(objectID)->passIndeces;

	if (passIndices[vkutil::MeshpassType::Forward] != -1)
	{
		Handle<PassObject> handle;
	    handle.handle = passIndices[vkutil::MeshpassType::Forward];

	    _forwardPass.objectToDelete.push_back(handle);
	    _forwardPass.unbatchedObjects.push_back(objectID);

	    passIndices[vkutil::MeshpassType::Forward] = -1;
	}

	if (passIndices[vkutil::MeshpassType::Transparency] != -1)
	{
		Handle<PassObject> handle;
	    handle.handle = passIndices[vkutil::MeshpassType::Transparency];

	    _transparentForwardPass.objectToDelete.push_back(handle);
	    _transparentForwardPass.unbatchedObjects.push_back(objectID);

	    passIndices[vkutil::MeshpassType::Transparency] = -1;
	}

	if (passIndices[vkutil::MeshpassType::DirectionalShadow] != -1)
	{
		Handle<PassObject> handle;
	    handle.handle = passIndices[vkutil::MeshpassType::DirectionalShadow];

	    _dirShadowPass.objectToDelete.push_back(handle);
	    _dirShadowPass.unbatchedObjects.push_back(objectID);

	    passIndices[vkutil::MeshpassType::DirectionalShadow] = -1;
	}

	if (passIndices[vkutil::MeshpassType::PointShadow] != -1)
	{
		Handle<PassObject> handle;
		handle.handle = passIndices[vkutil::MeshpassType::PointShadow];

		_pointShadowPass.objectToDelete.push_back(handle);
		_pointShadowPass.unbatchedObjects.push_back(objectID);

		passIndices[vkutil::MeshpassType::PointShadow] = -1;
	}

	if (passIndices[vkutil::MeshpassType::SpotShadow] != -1)
	{
		Handle<PassObject> handle;
		handle.handle = passIndices[vkutil::MeshpassType::SpotShadow];

		_spotShadowPass.objectToDelete.push_back(handle);
		_spotShadowPass.unbatchedObjects.push_back(objectID);

		passIndices[vkutil::MeshpassType::SpotShadow] = -1;
	}

	if (passIndices[vkutil::MeshpassType::Deferred] != -1)
	{
		Handle<PassObject> handle;
		handle.handle = passIndices[vkutil::MeshpassType::Deferred];

		_deferredPass.objectToDelete.push_back(handle);
		_deferredPass.unbatchedObjects.push_back(objectID);

		passIndices[vkutil::MeshpassType::Deferred] = -1;
	}

	if (get_renderable_object(objectID)->updateIndex == (uint32_t)-1)
	{
		get_renderable_object(objectID)->updateIndex = static_cast<uint32_t>(_dirtyObjects.size());

		_dirtyObjects.push_back(objectID);
	}
}

void RenderScene::update_transform(Handle<RenderableObject> objectID, const glm::mat4 transformMatrix)
{
	get_renderable_object(objectID)->transformMatrix = transformMatrix;
	update_object(objectID);
}

void RenderScene::clear_dirty_objects()
{
	for (auto& obj : _dirtyObjects)
	{
		get_renderable_object(obj)->updateIndex = (uint32_t)-1;
	}

	_dirtyObjects.clear();
}	

void RenderScene::fill_indirect_array(GPUIndirectObject* data, MeshPass& pass)
{
	int dataIndex = 0;
	for (int i = 0; i != pass.batches.size(); ++i)
	{
		auto batch = pass.batches[i];

		data[dataIndex].command.firstInstance = batch.first;
		data[dataIndex].command.instanceCount = 0;		// Will be filled in cull compute shader
		data[dataIndex].command.firstIndex = get_mesh(batch.meshID)->firstIndex;
		data[dataIndex].command.vertexOffset = get_mesh(batch.meshID)->firstVertex;
		data[dataIndex].command.indexCount = get_mesh(batch.meshID)->indexCount;
		data[dataIndex].batchID = i;
		data[dataIndex].objectID = 0;

		++dataIndex;
	}
}

void RenderScene::fill_instances_array(GPUInstance* data, MeshPass& pass)
{
	int dataIndex = 0;
	
	for (int i = 0; i != pass.batches.size(); ++i)
	{
		auto batch = pass.batches[i];

		for (int j = 0; j != batch.count; ++j)
		{
			data[dataIndex].batchID = i;
			data[dataIndex].objectID = pass.get(pass.flatBatches[j + batch.first].object)->original.handle;
			++dataIndex;
		}
	}
}

void RenderScene::fill_object_data(GPUObjectData* data)
{
	// I have to pass mapped buffer to this function. Buffer's size should be the same as renderables array size

	int dataIndex = 0;
	for (int i = 0; i != _renderables.size(); ++i)
	{
		Handle<RenderableObject> handle;
		handle.handle = i;
		write_object(data + i, handle);
	}
}

void RenderScene::write_object(GPUObjectData* target, Handle<RenderableObject> objectID)
{
	GPUObjectData tempData;
	RenderableObject* obj = get_renderable_object(objectID);
	tempData.model = obj->transformMatrix;
	Mesh* mesh = get_mesh(obj->meshID)->original;
	tempData.extents = glm::vec4(mesh->_bounds.extents, mesh->_bounds.radius);
	tempData.originRad = glm::vec4(mesh->_bounds.origin, mesh->_bounds.radius);
	tempData.baseColorTexId = obj->baseColorTexId;
	tempData.normalTexId = obj->normalTexId;
	tempData.armTexId = obj->armTexId;

	memcpy(target, &tempData, sizeof(GPUObjectData));
}

void RenderScene::merge_meshes(class VulkanEngine* engine)
{
    for (auto& mesh : _meshes)
    {
		mesh.firstIndex = _globalIndexBufferSize;
		mesh.firstVertex = _globalVertexBufferSize;

		_globalIndexBufferSize += mesh.original->_indices.size();
		_globalVertexBufferSize += mesh.original->_vertices.size();

		mesh.isMerged = true;
    }
    
	std::vector<assets::Vertex_f32_PNCV> vertices(_globalVertexBufferSize);
	std::vector<uint32_t> indices(_globalIndexBufferSize);

	size_t previousVerticesSize = 0;
	size_t previousIndicesSize = 0;

	for (int i = 0; i != _meshes.size(); ++i)
	{
		Mesh* mesh = _meshes[i].original;
	
		memcpy(vertices.data() + previousVerticesSize, mesh->_vertices.data(), mesh->_vertices.size() * sizeof(assets::Vertex_f32_PNCV));
	    previousVerticesSize += mesh->_vertices.size();

	    memcpy(indices.data() + previousIndicesSize, mesh->_indices.data(), mesh->_indices.size() * sizeof(uint32_t));
	    previousIndicesSize += mesh->_indices.size();
	}

	AllocatedBuffer vertexStagingBuffer(engine, _globalVertexBufferSize * sizeof(assets::Vertex_f32_PNCV),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	
	AllocatedBuffer indexStagingBuffer(engine, _globalIndexBufferSize * sizeof(uint32_t),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	vertexStagingBuffer.copy_from(engine, vertices.data(), _globalVertexBufferSize * sizeof(assets::Vertex_f32_PNCV));
	indexStagingBuffer.copy_from(engine, indices.data(), _globalIndexBufferSize * sizeof(uint32_t));
    
	_globalVertexBuffer = engine->create_buffer(_globalVertexBufferSize * sizeof(assets::Vertex_f32_PNCV),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	_globalIndexBuffer = engine->create_buffer(_globalIndexBufferSize * sizeof(uint32_t),
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY);

	engine->immediate_submit([&](VkCommandBuffer cmd){
	    AllocatedBuffer::copy_buffer_cmd(engine, cmd, &vertexStagingBuffer, &_globalVertexBuffer);
	    AllocatedBuffer::copy_buffer_cmd(engine, cmd, &indexStagingBuffer, &_globalIndexBuffer);
	});

	LOG_INFO("Vertex buffer size {}", _globalVertexBuffer._bufferSize)
	LOG_INFO("Index buffer size {}", _globalIndexBuffer._bufferSize)

	engine->_mainDeletionQueue.push_function([=](){
		vmaDestroyBuffer(engine->_allocator, _globalVertexBuffer._buffer, _globalVertexBuffer._allocation);
		vmaDestroyBuffer(engine->_allocator, _globalIndexBuffer._buffer, _globalIndexBuffer._allocation);
	});
	
	vmaDestroyBuffer(engine->_allocator, vertexStagingBuffer._buffer, vertexStagingBuffer._allocation);
	vmaDestroyBuffer(engine->_allocator, indexStagingBuffer._buffer, indexStagingBuffer._allocation);
}

void RenderScene::build_batches()
{
	// I have to read how async works
	//auto forward = std::async(std::launch::async, [&]{ refresh_pass(&_forwardPass); });
	auto deferred = std::async(std::launch::async, [&]{ refresh_pass(&_deferredPass);});
	auto shadow = std::async(std::launch::async, [&]{ refresh_pass(&_dirShadowPass); });
	auto pointShadow = std::async(std::launch::async, [&]{ refresh_pass(&_pointShadowPass); });
	auto spotShadow = std::async(std::launch::async, [&]{ refresh_pass(&_spotShadowPass); });
	auto transparent = std::async(std::launch::async, [&]{ refresh_pass(&_transparentForwardPass); });

	transparent.get();
	spotShadow.get();
	pointShadow.get();
	shadow.get();
	deferred.get();
	//forward.get();
}

void RenderScene::refresh_pass(RenderScene::MeshPass* meshPass)
{
	// I need to test if refresh passes works slow when I separate it into several functions 
	meshPass->needsIndirectRefresh = true;
	meshPass->needsInstanceRefresh = true;
	
	delete_batches(meshPass);

	std::vector<Handle<PassObject>> passObjectHandles;
	fill_pass_objects(meshPass, passObjectHandles);
	fill_flat_batches(meshPass, passObjectHandles);
	meshPass->batches.clear();
	
	meshPass->batches.reserve(meshPass->flatBatches.size());
	build_indirect_batches(meshPass, meshPass->batches, meshPass->flatBatches);
	meshPass->multibatches.clear();
	fill_multi_batches(meshPass);
}

void RenderScene::build_indirect_batches(RenderScene::MeshPass* pass, std::vector<IndirectBatch>& outBatches, std::vector<RenderScene::RenderBatch>& inObjects)
{
	RenderScene::IndirectBatch tempBatch;
	tempBatch.first = 0;
	tempBatch.count = 0;
	tempBatch.material = pass->get(inObjects[0].object)->material;
	tempBatch.meshID = pass->get(inObjects[0].object)->meshID;
	outBatches.push_back(tempBatch);
	
	auto* prevBatch = &outBatches.back();

	for (int i = 0; i != inObjects.size(); ++i)
	{
		RenderScene::PassObject* passObject = pass->get(inObjects[i].object);
		
		bool bSameMaterials = false;

		if (passObject->material.shaderPass == prevBatch->material.shaderPass)
		{
			bSameMaterials = true;
		}
		bool bSameMeshes = prevBatch->meshID.handle == passObject->meshID.handle;
		
		if (bSameMaterials && bSameMeshes)
		{
			++prevBatch->count;
		}
		else
		{
			tempBatch.first = i;
			tempBatch.count = 1;
			tempBatch.material = passObject->material;
			tempBatch.meshID = passObject->meshID;
			outBatches.push_back(tempBatch);

			prevBatch = &outBatches.back();
		}
	}		
}

RenderableObject* RenderScene::get_renderable_object(Handle<RenderableObject> objectID)
{
	return &_renderables[objectID.handle];
}

DrawMesh* RenderScene::get_mesh(Handle<DrawMesh> meshID)
{
	return &_meshes[meshID.handle];
}

vkutil::Material* RenderScene::get_material(Handle<vkutil::Material> materialID)
{
	return _materials[materialID.handle];
}

RenderScene::MeshPass* RenderScene::get_mesh_pass(vkutil::MeshpassType type)
{
	MeshPass* pass = nullptr;

	switch(type)
	{
		case vkutil::MeshpassType::Forward:
			pass = &_forwardPass;
			break;
		case vkutil::MeshpassType::DirectionalShadow:
			pass = &_dirShadowPass;
			break;
		case vkutil::MeshpassType::Transparency:
			pass = &_transparentForwardPass;
			break;
		case vkutil::MeshpassType::PointShadow:
			pass = &_pointShadowPass;
			break;
		case vkutil::MeshpassType::SpotShadow:
			pass = &_spotShadowPass;
			break;
		case vkutil::MeshpassType::None:
			LOG_ERROR("There are no mesh pass");
			break;
	}

	return pass;
}
	
Handle<DrawMesh> RenderScene::get_mesh_handle(Mesh* mesh)
{
	auto it = _meshConvert.find(mesh);
	Handle<DrawMesh> handle;

	if (it == _meshConvert.end())
	{
		uint32_t index = static_cast<uint32_t>(_meshes.size());
		handle.handle = index;
	
		DrawMesh temp;

		temp.original = mesh;
		temp.firstIndex = 0;
		temp.firstVertex = 0;
		temp.vertexCount = mesh->_vertices.size();
		temp.indexCount = mesh->_indices.size();

		_meshConvert[mesh] = handle;
		_meshes.push_back(temp);
	}
	else
	{
		handle = it->second;
	}

	return handle;
}

Handle<vkutil::Material> RenderScene::get_material_handle(vkutil::Material* material)
{
	auto it = _materialConvert.find(material);
	Handle<vkutil::Material> handle;

	if (it == _materialConvert.end())
	{
		uint32_t index = static_cast<uint32_t>(_materials.size());
		handle.handle = index;

		_materials.push_back(material);
		_materialConvert[material] = handle;
	}
	else
	{
		handle = it->second;
	}	
	 	
    return handle;
}	 	
	 	
//private methods

void RenderScene::delete_batches(MeshPass* meshPass)
{	
	if (meshPass->objectToDelete.size() > 0)
	{
		std::vector<RenderScene::RenderBatch> batchesToDelete;
		batchesToDelete.reserve(meshPass->objectToDelete.size());
	
		for (auto& handle : meshPass->objectToDelete)
		{
			meshPass->reusableObjects.push_back(handle);
		
			RenderScene::RenderBatch tempRenderBatch;

			tempRenderBatch.object = handle;

			RenderScene::PassObject passObject = meshPass->objects[handle.handle];
			
			uint64_t pipelineHash = std::hash<uint64_t>()((uint64_t)passObject.material.shaderPass->pipeline);
			uint64_t pipelineLayoutHash = std::hash<uint64_t>()((uint64_t)passObject.material.shaderPass->layout);
			uint32_t materialHash = static_cast<uint32_t>(pipelineHash ^ pipelineLayoutHash);
	
			uint32_t meshHash = uint64_t(materialHash) ^ uint64_t(passObject.meshID.handle);

			batchesToDelete.push_back(tempRenderBatch);

			meshPass->objects[handle.handle].meshID.handle = -1;
			meshPass->objects[handle.handle].original.handle = -1;
			meshPass->objects[handle.handle].material.shaderPass = nullptr;
			meshPass->objects[handle.handle].customKey = 0;
		}

		meshPass->objectToDelete.clear();
		
		std::sort(batchesToDelete.begin(), batchesToDelete.end(), [](RenderScene::RenderBatch& first, RenderScene::RenderBatch& second){
			if (first.sortKey < second.sortKey) { return true; }
			else if (first.sortKey == second.sortKey) { return first.object.handle < second.object.handle; }
			else { return false; }
		});

		std::vector<RenderScene::RenderBatch> newFlatBatches;
		newFlatBatches.reserve(batchesToDelete.size());

		std::set_difference(meshPass->flatBatches.begin(), meshPass->flatBatches.end(),
			batchesToDelete.begin(), batchesToDelete.end(), std::back_inserter(newFlatBatches),
			[](RenderScene::RenderBatch& first, RenderScene::RenderBatch& second){
				if (first.sortKey < second.sortKey) { return true; }
				else if (first.sortKey == second.sortKey) { return first.object.handle < second.object.handle; }
				else { return false; }
			});
	}
}

void RenderScene::fill_pass_objects(MeshPass* meshPass, std::vector<Handle<PassObject>>& passObjectsHandles)
{
	passObjectsHandles.reserve(meshPass->unbatchedObjects.size());

	for (auto& handle : meshPass->unbatchedObjects)
	{
		RenderableObject* renderableObject = get_renderable_object(handle);
		vkutil::Material* material = get_material(renderableObject->material);
		
		PassMaterial tempMaterialPass;
		tempMaterialPass.shaderPass = material->original->passShaders[meshPass->type];
		//tempMaterialPass.materialSet = material->passSets[meshPass->type];
		
		PassObject tempObject;
		tempObject.customKey = renderableObject->customSortKey;
		tempObject.material = tempMaterialPass;
		tempObject.meshID = renderableObject->meshID;
		tempObject.original = handle;
		
		Handle<PassObject> tempHandle;
		if (meshPass->reusableObjects.size() > 0)
		{
			tempHandle = meshPass->reusableObjects.back();
			meshPass->reusableObjects.pop_back();
			meshPass->objects[tempHandle.handle] = tempObject;
		}
		else
		{
			tempHandle.handle = meshPass->objects.size();
			meshPass->objects.push_back(tempObject);
		}
		
		passObjectsHandles.push_back(tempHandle);
		get_renderable_object(handle)->passIndeces[meshPass->type] = tempHandle.handle;
	}

	meshPass->unbatchedObjects.clear();
}

void RenderScene::fill_flat_batches(MeshPass* meshPass, std::vector<Handle<PassObject>>& passObjectsHandles)
{
	std::vector<RenderScene::RenderBatch> newFlatBatches;

	for (auto& handle : passObjectsHandles)
	{
		RenderScene::RenderBatch tempRenderBatch;
		tempRenderBatch.object = handle;

		auto passObject = meshPass->objects[handle.handle];

		uint64_t pipelineHash = std::hash<uint64_t>()((uint64_t)passObject.material.shaderPass->pipeline);
		uint64_t pipelineLayoutHash = std::hash<uint64_t>()((uint64_t)passObject.material.shaderPass->layout);
		uint32_t materialHash = static_cast<uint32_t>(pipelineHash ^ pipelineLayoutHash);

		uint32_t meshHash = uint64_t(materialHash) ^ uint64_t(passObject.meshID.handle);
		
		tempRenderBatch.sortKey = uint64_t(meshHash) | (uint64_t(passObject.customKey) << 32);

		newFlatBatches.push_back(tempRenderBatch);
	}

	std::sort(newFlatBatches.begin(), newFlatBatches.end(), [](RenderScene::RenderBatch& first, RenderScene::RenderBatch& second){
		if (first.sortKey < second.sortKey) { return true; }
		else if (first.sortKey == second.sortKey) { return first.object.handle < second.object.handle; }
		else { return false; }
	});

	if (meshPass->flatBatches.size() > 0 && newFlatBatches.size() > 0)
	{
		size_t oldEnd = meshPass->flatBatches.size();
		meshPass->flatBatches.reserve(oldEnd + newFlatBatches.size());

		for (auto& batch : newFlatBatches)
		{
			meshPass->flatBatches.push_back(batch);
		}

		RenderScene::RenderBatch* begin = meshPass->flatBatches.data();
		RenderScene::RenderBatch* mid = begin + oldEnd;
		RenderScene::RenderBatch* end = begin + meshPass->flatBatches.size();

		std::inplace_merge(begin, mid, end, [](RenderScene::RenderBatch first, RenderScene::RenderBatch second){
			if (first.sortKey < second.sortKey) { return true; }
			else if (first.sortKey == second.sortKey) { return first.object.handle < second.object.handle; }
			else { return false; }
		});
	}
	else
	{
		meshPass->flatBatches = std::move(newFlatBatches);
	}
}

void RenderScene::fill_multi_batches(MeshPass* meshPass)
{	
	RenderScene::MultiBatch tempMultiBatch;
	tempMultiBatch.count = 1;
	tempMultiBatch.first = 0;

	for (int i = 1; i != meshPass->batches.size(); ++i)
	{
		RenderScene::IndirectBatch* currentBatch = &meshPass->batches[i];
		RenderScene::IndirectBatch* prevBatch = &meshPass->batches[tempMultiBatch.first];

		bool bCompatibleMesh = get_mesh(currentBatch->meshID)->isMerged;

		bool bSameMaterials = false;

		if (bCompatibleMesh && currentBatch->material.shaderPass == prevBatch->material.shaderPass)
		{
			bSameMaterials = true;
		}

		if (!bSameMaterials || !bCompatibleMesh)
		{
			meshPass->multibatches.push_back(tempMultiBatch);
			tempMultiBatch.count = 1;
 			tempMultiBatch.first = i;
		}
		else
		{
			++tempMultiBatch.count;
		}
	}

	meshPass->multibatches.push_back(tempMultiBatch);
}
		
RenderScene::PassObject* RenderScene::MeshPass::get(Handle<RenderScene::PassObject> handle)
{
	return &objects[handle.handle];
}

