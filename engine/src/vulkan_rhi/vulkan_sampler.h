#pragma once

#include "vulkan_descriptor.h"
#include "vulkan_object.h"
#include "rhi/resources.h"
#include <vulkan/vulkan.h>

namespace ad_astris::vulkan
{
	class VulkanSampler : public VulkanDescriptor, public IVulkanObject
	{
		public:
			VulkanSampler() = default;
			VulkanSampler(VulkanDevice* device, VkSamplerCreateInfo& info);
			VulkanSampler(VulkanDevice* device, rhi::SamplerInfo* samplerInfo);
		
			void destroy(VulkanDevice* device) override;
			VkSampler get_handle() { return _sampler; }

		private:
			VkSampler _sampler{ VK_NULL_HANDLE };

			void parse_sampler_info(rhi::SamplerInfo* samplerInfo, VkSamplerCreateInfo& outCreateInfo);
	};
}
