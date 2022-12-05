#include "vk_descriptors.h"
#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_scene.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include <future>

inline uint32_t get_group_count(uint32_t threadCount, uint32_t localSize)
{
	return (threadCount + localSize - 1) / localSize;
}

void VulkanEngine::fill_renderable_objects()
{
	// I have to think about removing RenderObject data type and make MeshObject vector in init_scene method
	std::vector<MeshObject> meshObjects;

	for (auto& obj : _renderables)
	{
		MeshObject temp;
		temp.mesh = obj.mesh;
		temp.material = obj.material;
		temp.transformMatrix = obj.transformMatrix;
		temp.customSortKey = 1;

		meshObjects.push_back(temp);
	}

	_renderScene.register_object_batch(meshObjects.data(), meshObjects.size());
	_renderScene.build_batches();
}

void VulkanEngine::draw_forward_pass()
{
	
}

void VulkanEngine::culling(RenderScene::MeshPass& meshPass, VkCommandBuffer cmd)
{
	
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

			AllocatedBufferT<GPUInstance>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &pass.passObjectsBuffer);

			if (pass.passObjectsBuffer._buffer != VK_NULL_HANDLE)
			{
				get_current_frame()._frameDeletionQueue.push_function([=](){
					pPass->passObjectsBuffer.destroy_buffer(this);
				});
			}

			unmapMemory.push_back(tempBuffer);
			
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
	VkImageMemoryBarrier mainDepthBarrier = vkinit::image_barrier(_depthImage._imageData._image,
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
			sourceTarget.imageView = _offscrDepthImage._imageView;
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

		VkImageMemoryBarrier reduceBarrier = vkinit::image_barrier(_offscrDepthImage._imageData._image,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &reduceBarrier);
	}

	mainDepthBarrier = vkinit::image_barrier(_depthImage._imageData._image,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &mainDepthBarrier);
}
