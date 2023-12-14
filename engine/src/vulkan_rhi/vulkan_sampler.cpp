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
	parse_sampler_info(samplerInfo, createInfo);
	VK_CHECK(vkCreateSampler(device->get_device(), &createInfo, nullptr, &_sampler));
}

void VulkanSampler::destroy(VulkanDevice* device)
{
	if (_sampler != VK_NULL_HANDLE)
		vkDestroySampler(device->get_device(), _sampler, nullptr);
	_sampler = VK_NULL_HANDLE;
}

void VulkanSampler::parse_sampler_info(rhi::SamplerInfo* samplerInfo, VkSamplerCreateInfo& outCreateInfo)
{
	outCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	get_filter(samplerInfo->filter, outCreateInfo);
	outCreateInfo.addressModeU = get_address_mode(samplerInfo->addressMode);
	outCreateInfo.addressModeV = outCreateInfo.addressModeU;
	outCreateInfo.addressModeW = outCreateInfo.addressModeU;
	outCreateInfo.minLod = samplerInfo->minLod;
	outCreateInfo.maxLod = samplerInfo->maxLod;
	if (outCreateInfo.anisotropyEnable == VK_TRUE)
		outCreateInfo.maxAnisotropy = samplerInfo->maxAnisotropy;
	if (samplerInfo->borderColor != rhi::BorderColor::UNDEFINED)
		outCreateInfo.borderColor = get_border_color(samplerInfo->borderColor);

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
			outCreateInfo.pNext = &reductionMode;
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
			outCreateInfo.pNext = &reductionMode;
			break;
	}

}
