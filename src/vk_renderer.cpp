#include <iostream>

#include <cmath>
#include "engine_actors.h"
#include "fmt/core.h"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "material_system.h"
#include "vk_descriptors.h"
#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_scene.h"
#include <memory>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include <future>

inline uint32_t get_group_count(uint32_t threadCount, uint32_t localSize)
{
	return (threadCount + localSize - 1) / localSize;
}

inline glm::vec4 normalize_plane(glm::vec4 p)
{
	return p / glm::length(glm::vec3(p));
}

inline glm::mat4 perspectiveProjection(float fovY, float aspectWbyH, float zNear)
{
	float f = 1.0f / std::tan(fovY / 2.0f);
	return glm::mat4(
		f / aspectWbyH, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, zNear, 0.0f);
}

void VulkanEngine::fill_renderable_objects()
{
	_renderScene.register_object_batch(_meshObjects.data(), _meshObjects.size());
	
	_renderScene.build_batches();
	_renderScene.merge_meshes(this);
}

void VulkanEngine::draw_forward_pass(VkCommandBuffer cmd)
{
	auto& meshPass = _renderScene._forwardPass;

	VkDescriptorImageInfo samplerInfo{};
	samplerInfo.sampler = _textureSampler;

	VkDescriptorBufferInfo objectDataInfo = _renderScene._objectDataBuffer.get_info(true);
	VkDescriptorBufferInfo instanceInfo = meshPass.compactedInstanceBuffer.get_info(true);
	VkDescriptorBufferInfo cameraInfo = get_current_frame()._cameraBuffer.get_info();
	VkDescriptorBufferInfo sceneDataInfo = get_current_frame()._sceneDataBuffer.get_info();
	VkDescriptorBufferInfo dirLightsInfo = _renderScene._dirLightsBuffer.get_info(true);
	VkDescriptorBufferInfo pointLightsInfo = _renderScene._pointLightsBuffer.get_info(true);
	VkDescriptorBufferInfo spotLightsInfo = _renderScene._spotLightsBuffer.get_info(true);

	VkDescriptorSet globalDescriptorSet;
	VkDescriptorSet objectDescriptorSet;
	VkDescriptorSet texturesDescriptorSet1;
	VkDescriptorSet texturesDescriptorSet2;
	VkDescriptorSet texturesDescriptorSet3;

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &dirLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(1, &pointLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(2, &spotLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(3, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(4, &sceneDataInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(5, &samplerInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(globalDescriptorSet);

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &objectDataInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(1, &instanceInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build(objectDescriptorSet);

	size_t descriptorsAmount = _renderScene._baseColorInfos.size() + _renderScene._normalInfos.size() + _renderScene._armInfos.size();
	
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._baseColorInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._baseColorInfos.size())
		.build_non_uniform(texturesDescriptorSet1, _renderScene._baseColorInfos.size());
		
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._normalInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._normalInfos.size())
		.build_non_uniform(texturesDescriptorSet2, _renderScene._normalInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._armInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._armInfos.size())
		.build_non_uniform(texturesDescriptorSet3, _renderScene._armInfos.size());

	std::vector<VkDescriptorSet> sets = { globalDescriptorSet, objectDescriptorSet, texturesDescriptorSet1, texturesDescriptorSet2, texturesDescriptorSet3 };

	vkutil::ShaderPass* prevMaterial = nullptr;

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_renderScene._globalVertexBuffer._buffer, &offset);
	vkCmdBindIndexBuffer(cmd, _renderScene._globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
	
	for (int i = 0; i != meshPass.multibatches.size(); ++i)
	{
		auto& multibatch = meshPass.multibatches[i];
		auto& batch = meshPass.batches[multibatch.first];

		auto renderPass = batch.material.shaderPass;

		if (renderPass != prevMaterial)
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->pipeline);
			for (int i = 0; i != sets.size(); ++i)
			{
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass->layout, i, 1, &sets[i], 0, nullptr);
			}
			prevMaterial = renderPass;
		}

		VkDeviceSize offset = multibatch.first * sizeof(GPUIndirectObject);
		uint32_t stride = sizeof(GPUIndirectObject);

		auto& meshPass = _renderScene._forwardPass;

		vkCmdDrawIndexedIndirect(cmd, meshPass.drawIndirectBuffer._buffer, offset, multibatch.count, stride);
	}
}

void VulkanEngine::culling(RenderScene::MeshPass& meshPass, VkCommandBuffer cmd, CullParams cullParams)
{
	VkDescriptorBufferInfo objectDataBufferInfo = _renderScene._objectDataBuffer.get_info(true);
	VkDescriptorBufferInfo cameraBufferInfo = get_current_frame()._cameraBuffer.get_info();
	VkDescriptorBufferInfo drawIndirectBufferInfo = meshPass.drawIndirectBuffer.get_info(true);
	VkDescriptorBufferInfo gpuInstanceBufferInfo = meshPass.passObjectsBuffer.get_info(true);
	VkDescriptorBufferInfo finalInstanceBufferInfo = meshPass.compactedInstanceBuffer.get_info(true);

	VkDescriptorImageInfo depthPyramidImageInfo;
	depthPyramidImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	depthPyramidImageInfo.imageView = _depthPyramid.imageView;
	depthPyramidImageInfo.sampler = _depthSampler;

	VkDescriptorSet cullingSet;
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &objectDataBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(1, &drawIndirectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(2, &gpuInstanceBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(3, &finalInstanceBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_image(4, &depthPyramidImageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(5, &cameraBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.build(cullingSet);

	glm::mat4 projection = camera.get_projection_matrix((float)_windowExtent.width, (float)_windowExtent.height);
	glm::mat4 projectionT = glm::transpose(projection);

	glm::vec4 frustumX = normalize_plane(projectionT[3] + projectionT[0]);
	glm::vec4 frustumY = normalize_plane(projectionT[3] + projectionT[1]);
		
	CullData cullData;
	cullData.P00 = projection[0][0];
	cullData.P11 = projection[1][1];
	cullData.znear = 0.001f;
	cullData.zfar = cullParams.drawDist;
	cullData.frustum[0] = frustumX.x;
	cullData.frustum[1] = frustumX.z;
	cullData.frustum[2] = frustumY.y;
	cullData.frustum[3] = frustumY.z;
	cullData.drawCount = static_cast<uint32_t>(meshPass.flatBatches.size());
	cullData.cullingEnabled = cullParams.frustumCull;
	cullData.lodEnabled = false;
	cullData.occlusionEnabled = cullParams.occlusionCull;
	cullData.lodBase = 10.0f;
	cullData.lodStep = 1.5f;
	cullData.pyramidWidth = static_cast<uint32_t>(_depthPyramidWidth);
	cullData.pyramidHeight = static_cast<uint32_t>(_depthPyramidHeight);
	cullData.view = camera.get_view_matrix();

	cullData.AABBcheck = cullParams.aabb;
	cullData.aabbmin_x = cullParams.aabbmin.x;
	cullData.aabbmin_y = cullParams.aabbmin.y;
	cullData.aabbmin_z = cullParams.aabbmin.z;
	cullData.aabbmax_x = cullParams.aabbmax.x;
	cullData.aabbmax_y = cullParams.aabbmax.y;
	cullData.aabbmax_z = cullParams.aabbmax.z;

	if (cullParams.drawDist > 10000)
	{
		cullData.distCull = false;
	}
	else
	{
		cullData.distCull = true;
	}

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _cullingPipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _cullintPipelineLayout, 0,1, &cullingSet, 0, nullptr);
	vkCmdPushConstants(cmd, _cullintPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullData), &cullData);
	vkCmdDispatch(cmd, static_cast<uint32_t>((meshPass.flatBatches.size() / 256) + 1), 1, 1);

	VkBufferMemoryBarrier barrier1 = vkinit::buffer_barrier(&meshPass.compactedInstanceBuffer, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, _graphicsQueueFamily);
	_afterCullingBufferBarriers.push_back(barrier1);

	VkBufferMemoryBarrier barrier2 = vkinit::buffer_barrier(&meshPass.drawIndirectBuffer, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, _graphicsQueueFamily);
	_afterCullingBufferBarriers.push_back(barrier2);
}

void VulkanEngine::prepare_data_for_drawing(VkCommandBuffer cmd)
{
	RenderScene* scene = &_renderScene;

	size_t allocSize = scene->_renderables.size() * sizeof(GPUObjectData);
	if (allocSize != scene->_objectDataBuffer._bufferSize)
	{
		reallocate_buffer(scene->_objectDataBuffer, allocSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		void* data;
		vmaMapMemory(_allocator, scene->_objectDataBuffer._allocation, &data);
		scene->fill_object_data(reinterpret_cast<GPUObjectData*>(data));
		vmaUnmapMemory(_allocator, scene->_objectDataBuffer._allocation);

		scene->_dirtyObjects.clear();
	}

	if (scene->_renderables.size() * 0.8f >= scene->_dirtyObjects.size())
	{
		void* data;
		vmaMapMemory(_allocator, scene->_objectDataBuffer._allocation, &data);

		for (auto& handle : scene->_dirtyObjects)
		{
			uint32_t offset = handle.handle;
			scene->write_object(reinterpret_cast<GPUObjectData*>(data) + offset, handle);
		}

		vmaUnmapMemory(_allocator, scene->_objectDataBuffer._allocation);

		scene->_dirtyObjects.clear();
	}
	else
	{
		AllocatedBufferT<GPUObjectData> tempBuffer(this, allocSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* data;
		vmaMapMemory(_allocator, tempBuffer._allocation, &data);
		scene->fill_object_data(reinterpret_cast<GPUObjectData*>(data));
		vmaUnmapMemory(_allocator, tempBuffer._allocation);
		
		get_current_frame()._frameDeletionQueue.push_function([=](){
			scene->_objectDataBuffer.destroy_buffer(this);
		});

		AllocatedBufferT<GPUObjectData>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_objectDataBuffer);
		scene->_dirtyObjects.clear();
	}

	size_t dirLightsBufSize = sizeof(actors::DirectionLight) * scene->_dirLights.size();
	size_t pointLightsBufSize = sizeof(actors::PointLight) * scene->_pointLights.size();
	size_t spotLightsBufSize = sizeof(actors::SpotLight) * scene->_spotLights.size();

	if (dirLightsBufSize != scene->_dirLightsBuffer._bufferSize)
	{
		reallocate_buffer(scene->_dirLightsBuffer, dirLightsBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		AllocatedBufferT<actors::DirectionLight> tempBuffer(this, dirLightsBufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		tempBuffer.copy_from(this, scene->_dirLights.data(), dirLightsBufSize);
		immediate_submit([&](VkCommandBuffer cmd){
			AllocatedBufferT<actors::DirectionLight>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_dirLightsBuffer);
		});
		tempBuffer.destroy_buffer(this);
	}
	if (spotLightsBufSize != scene->_spotLightsBuffer._bufferSize)
	{
		reallocate_buffer(scene->_spotLightsBuffer, spotLightsBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		AllocatedBufferT<actors::SpotLight> tempBuffer(this, spotLightsBufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		tempBuffer.copy_from(this, scene->_spotLights.data(), spotLightsBufSize);
		immediate_submit([&](VkCommandBuffer cmd){
			AllocatedBufferT<actors::SpotLight>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_spotLightsBuffer);
		});
		tempBuffer.destroy_buffer(this);
	}
	if (pointLightsBufSize != scene->_pointLightsBuffer._bufferSize)
	{
		reallocate_buffer(scene->_pointLightsBuffer, pointLightsBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		AllocatedBufferT<actors::PointLight> tempBuffer(this, pointLightsBufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		tempBuffer.copy_from(this, scene->_pointLights.data(), pointLightsBufSize);
		immediate_submit([&](VkCommandBuffer cmd){
			AllocatedBufferT<actors::PointLight>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_pointLightsBuffer);
		});
		//AllocatedBufferT<actors::PointLight>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_pointLightsBuffer);
		tempBuffer.destroy_buffer(this);
	}
	
	std::vector<RenderScene::MeshPass*> passes = {
		&_renderScene._forwardPass,
		&_renderScene._shadowPass,
		&_renderScene._transparentForwardPass
	};

	for (int i = 0; i != passes.size(); ++i)
	{
		auto& pass = *passes[i];

		size_t compactedBufSize = pass.flatBatches.size() * sizeof(uint32_t);
		size_t passObjBufSize = pass.flatBatches.size() * sizeof(GPUInstance);
		size_t indirectBufSize = pass.batches.size() * sizeof(GPUIndirectObject);
		
		if (pass.compactedInstanceBuffer._bufferSize != compactedBufSize)
		{
			reallocate_buffer(pass.compactedInstanceBuffer, compactedBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}
		if (pass.passObjectsBuffer._bufferSize != passObjBufSize)
		{
			reallocate_buffer(pass.passObjectsBuffer, passObjBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}
		if (pass.drawIndirectBuffer._bufferSize != indirectBufSize)
		{
			reallocate_buffer(pass.drawIndirectBuffer, indirectBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}
	}

	std::vector<std::future<void>> asyncCalls;
	asyncCalls.reserve(passes.size() * 3);

	std::vector<AllocatedBuffer> unmapMemory;
	std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;

	for (int i = 0; i != passes.size(); ++i)
	{
		auto& pass = *passes[i];
		auto pPass = passes[i];
		
		if (pass.batches.size() > 0 && pass.needsIndirectRefresh)
		{
			AllocatedBufferT<GPUIndirectObject> tempBuffer(this, pass.drawIndirectBuffer._bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU);

			void* data;
			vmaMapMemory(_allocator, tempBuffer._allocation, &data);

			asyncCalls.push_back(std::async(std::launch::async, [=]{
				scene->fill_indirect_array(reinterpret_cast<GPUIndirectObject*>(data), *pPass);
			}));

			if (pass.clearIndirectBuffer._buffer != VK_NULL_HANDLE)
			{
				get_current_frame()._frameDeletionQueue.push_function([=](){
					pPass->clearIndirectBuffer.destroy_buffer(this);
				});
			}

			unmapMemory.push_back(tempBuffer);

			pass.needsIndirectRefresh = false;
			pass.clearIndirectBuffer = tempBuffer;			
		}

		if (pass.flatBatches.size() > 0 && pass.needsInstanceRefresh)
		{
			AllocatedBufferT<GPUInstance> tempBuffer(this, pass.passObjectsBuffer._bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU);

			void* data;
			vmaMapMemory(_allocator, tempBuffer._allocation, &data);
			asyncCalls.push_back(std::async(std::launch::async, [=](){
				scene->fill_instances_array(reinterpret_cast<GPUInstance*>(data), *pPass);
			}));

			unmapMemory.push_back(tempBuffer);

			get_current_frame()._frameDeletionQueue.push_function([=](){
				vmaDestroyBuffer(_allocator, tempBuffer._buffer, tempBuffer._allocation);
			});

			AllocatedBufferT<GPUInstance>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &pass.passObjectsBuffer);

			VkBufferMemoryBarrier barrier = vkinit::buffer_barrier(&pass.passObjectsBuffer,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
				_graphicsQueueFamily);

			bufferMemoryBarriers.push_back(barrier);

			pass.needsInstanceRefresh = false;
		}
	}

	for (auto& call : asyncCalls)
	{
		call.get();
	}

	for (auto& unmap : unmapMemory)
	{
		vmaUnmapMemory(_allocator, unmap._allocation);
	}

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0,
		nullptr, static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(), 0, nullptr);

	bufferMemoryBarriers.clear();
}

void VulkanEngine::depth_reduce(VkCommandBuffer cmd)
{
	// Method to create depth pyramid based on the depth map mipmaps
	VkImageMemoryBarrier mainDepthBarrier = vkinit::image_barrier(_offscrDepthImage.imageData.image,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &mainDepthBarrier);
	
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _depthReducePipeline);
	
	for (int32_t i = 0; i != _depthPyramidLevels; ++i)
	{
		VkDescriptorImageInfo destTarget;
		destTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		destTarget.imageView = _depthPyramideMips[i];
		destTarget.sampler = _depthSampler;

		VkDescriptorImageInfo sourceTarget;
		sourceTarget.sampler = _depthSampler;

		if (i == 0)
		{
			sourceTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			sourceTarget.imageView = _offscrDepthImage.imageView;
		}
		else
		{
			sourceTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			sourceTarget.imageView = _depthPyramideMips[i - 1];
		}

		VkDescriptorSet depthSet;
		vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
			.bind_image(0, &destTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.bind_image(1, &sourceTarget, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build(depthSet);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _depthReduceLayout, 0, 1, &depthSet, 0, nullptr);

		uint32_t levelWidth = _depthPyramidWidth >> i;
		uint32_t levelHeight = _depthPyramidHeight >> i;
		if (levelWidth < 1) levelWidth = 1;
		if (levelHeight < 1) levelHeight = 1;

		DepthReduceData reduceData = { glm::vec2(levelWidth, levelHeight) };
		vkCmdPushConstants(cmd, _depthReduceLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(DepthReduceData), &reduceData);
		vkCmdDispatch(cmd, get_group_count(levelWidth, 32), get_group_count(levelHeight, 32), 1);

		VkImageMemoryBarrier reduceBarrier = vkinit::image_barrier(_depthPyramid.imageData.image,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &reduceBarrier);
	}

	mainDepthBarrier = vkinit::image_barrier(_offscrDepthImage.imageData.image,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &mainDepthBarrier);
}

void VulkanEngine::prepare_gpu_indirect_buffer(VkCommandBuffer cmd, RenderScene::MeshPass& meshPass)
{
	AllocatedBuffer::copy_buffer_cmd(this, cmd, &meshPass.clearIndirectBuffer, &meshPass.drawIndirectBuffer);
	VkBufferMemoryBarrier memoryBarrier = vkinit::buffer_barrier(&meshPass.drawIndirectBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, _graphicsQueueFamily);
	_beforeCullingBufferBarriers.push_back(memoryBarrier);
}
