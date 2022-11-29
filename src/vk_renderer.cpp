#include "vk_descriptors.h"
#include "vk_engine.h"
#include "vk_scene.h"
#include <vulkan/vulkan_core.h>

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

void VulkanEngine::culling(RenderScene::MeshPass& meshPass, VkCommandBuffer cmd)
{
	std::vector<GPUIndirectObject> gpuIndirectObjects;
	gpuIndirectObjects.reserve(meshPass.batches.size());
	_renderScene.fill_indirect_array(gpuIndirectObjects.data(), meshPass);

	std::vector<GPUInstance> gpuInstances;
	gpuInstances.reserve(meshPass.batches.size());
	_renderScene.fill_instances_array(gpuInstances.data(), meshPass);

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

		VkImageMemoryBarrier reduceBarrier{};
		reduceBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		reduceBarrier.pNext = nullptr;
		reduceBarrier.image = _offscrDepthImage._imageData._image;
		reduceBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		reduceBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		reduceBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		reduceBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		reduceBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		reduceBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		reduceBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		reduceBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		reduceBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &reduceBarrier);
	}
}

void VulkanEngine::draw_forward_pass()
{
	
}
