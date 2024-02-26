#include "vulkan_sampler.h"
#include "vulkan_common.h"

using namespace ad_astris::vulkan;

VulkanSampler::VulkanSampler(VulkanDevice* device, VkSamplerCreateInfo& info)
{
	VK_CHECK(vkCreateSampler(device->get_device(), &info, nullptr, &_sampler));
}

VulkanSampler::VulkanSampler(VulkanDevice* device, rhi::SamplerInfo* samplerInfo)
{
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	get_filter(samplerInfo->filter, createInfo);
	createInfo.addressModeU = get_address_mode(samplerInfo->addressMode);
	createInfo.addressModeV = createInfo.addressModeU;
	createInfo.addressModeW = createInfo.addressModeU;
	createInfo.minLod = samplerInfo->minLod;
	createInfo.maxLod = VK_LOD_CLAMP_NONE;
	createInfo.mipLodBias = 0.0f;
	if (createInfo.anisotropyEnable == VK_TRUE)
		createInfo.maxAnisotropy = samplerInfo->maxAnisotropy;
	if (samplerInfo->borderColor != rhi::BorderColor::UNDEFINED)
		createInfo.borderColor = get_border_color(samplerInfo->borderColor);

	// Using of min max sampler filter
	VkSamplerReductionModeCreateInfo reductionMode{};
	reductionMode.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
	switch (samplerInfo->filter)
	{
		case rhi::Filter::MINIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::Filter::MINIMUM_ANISOTROPIC:
			reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
			createInfo.pNext = &reductionMode;
			break;
		case rhi::Filter::MAXIMUM_MIN_MAG_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_NEAREST_MAG_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_NEAREST:
		case rhi::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
		case rhi::Filter::MAXIMUM_ANISOTROPIC:
			reductionMode.reductionMode = VK_SAMPLER_REDUCTION_MODE_MAX;
			createInfo.pNext = &reductionMode;
			break;
	}
	VK_CHECK(vkCreateSampler(device->get_device(), &createInfo, nullptr, &_sampler));
}

void VulkanSampler::destroy(VulkanDevice* device)
{
	if (_sampler != VK_NULL_HANDLE)
		vkDestroySampler(device->get_device(), _sampler, nullptr);
	_sampler = VK_NULL_HANDLE;
}
