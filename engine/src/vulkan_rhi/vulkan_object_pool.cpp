#include "vulkan_object_pool.h"
#include "vulkan_device.h"
#include "vulkan_common.h"

using namespace ad_astris::vulkan;

VulkanObjectPool::VulkanObjectPool()
{
	create_pool_for_new_resource<VulkanPipeline>(1024);
	create_pool_for_new_resource<VulkanShader>(1024);
	create_pool_for_new_resource<VulkanBuffer>(512);
	create_pool_for_new_resource<VulkanTexture>(256);
	create_pool_for_new_resource<VulkanTextureView>(512);
	create_pool_for_new_resource<VulkanSampler>(256);
	create_pool_for_new_resource<VulkanRenderPass>(4);
}

void VulkanObjectPool::cleanup(VulkanDevice* device)
{
	for (auto it = _vkObjects.rbegin(); it != _vkObjects.rend(); ++it)
	{
		(*it)->destroy(device);
	}
	ObjectPool::cleanup();
}
