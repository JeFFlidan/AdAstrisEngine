#pragma once

#include "vulkan_api.h"
#include "vulkan_descriptor.h"
#include "vulkan_object.h"
#include "rhi/resources.h"

namespace ad_astris::vulkan
{
	class VulkanBufferView : public VulkanDescriptor, public IVulkanObject
	{
		public:
			VulkanBufferView();
			VulkanBufferView(VulkanDevice* device, rhi::BufferViewInfo* bufferViewInfo, rhi::Buffer* buffer);
			VulkanBufferView(rhi::Buffer* buffer, uint64_t size, uint64_t offset);

			void create(VulkanDevice* device, rhi::BufferViewInfo* bufferViewInfo, rhi::Buffer* buffer);
			void create(rhi::Buffer* buffer, uint64_t size, uint64_t offset);
			void destroy(VulkanDevice* device) override;

			VkBufferView get_handle() const
			{
				return std::visit([&](auto& arg)->VkBufferView
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, VkBufferView>)
						return arg;
					LOG_FATAL("VulkanBufferView::get_handle(): Failed to get VkBufferView")
					return VkBufferView{};
				}, _bufferView);
			}

			const VkDescriptorBufferInfo& get_descriptor_info() const
			{
				return std::visit([&](auto& arg)->const VkDescriptorBufferInfo&
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, VkDescriptorBufferInfo>)
						return arg;
					LOG_FATAL("VulkanBufferView::get_handle(): Failed to get RawBufferDescriptorInfo")
					return VkDescriptorBufferInfo{};
				}, _bufferView);
			}

			bool is_raw() const
			{
				return std::visit([&](auto& arg)->bool
				{
					using ArgType = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<ArgType, VkDescriptorBufferInfo>)
						return true;
					return false;
				}, _bufferView);
			}
		
		private:
			std::variant<VkBufferView, VkDescriptorBufferInfo> _bufferView;
	};
}
