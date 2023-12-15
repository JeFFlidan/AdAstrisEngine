#include "vulkan_query_pool.h"
#include "vulkan_device.h"
#include "vulkan_common.h"

using namespace ad_astris::vulkan;

VulkanQueryPool::VulkanQueryPool(VulkanDevice* device, rhi::QueryPoolInfo* queryPoolInfo)
{
	VkQueryPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	createInfo.queryCount = queryPoolInfo->queryCount;
	createInfo.queryType = get_query_type(queryPoolInfo->type);

	if (createInfo.queryType == VK_QUERY_TYPE_PIPELINE_STATISTICS)
	{
		createInfo.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
			| VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
			| VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
	}

	VK_CHECK(vkCreateQueryPool(device->get_device(), &createInfo, nullptr, &_queryPool));
}

void VulkanQueryPool::destroy(VulkanDevice* device)
{
	if (_queryPool != VK_NULL_HANDLE)
		vkDestroyQueryPool(device->get_device(), _queryPool, nullptr);
}
