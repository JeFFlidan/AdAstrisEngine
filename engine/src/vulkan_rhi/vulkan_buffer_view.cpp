#include "vulkan_device.h"
#include "vulkan_common.h"
#include "vulkan_buffer_view.h"
#include "rhi/utils.h"

using namespace ad_astris;
using namespace vulkan;

VulkanBufferView::VulkanBufferView() : _bufferView(VkBufferView{ VK_NULL_HANDLE })
{
	
}

VulkanBufferView::VulkanBufferView(VulkanDevice* device, rhi::BufferViewInfo* bufferViewInfo, rhi::Buffer* buffer)
	: _bufferView(VkBufferView{ VK_NULL_HANDLE })
{
	create(device, bufferViewInfo, buffer);
}

VulkanBufferView::VulkanBufferView(rhi::Buffer* buffer, uint64_t size, uint64_t offset)
	: _bufferView(VkDescriptorBufferInfo{ get_vk_obj(buffer)->get_handle(), offset, size })
{
	
}

void VulkanBufferView::create(VulkanDevice* device, rhi::BufferViewInfo* bufferViewInfo, rhi::Buffer* buffer)
{
	rhi::Format viewFormat = buffer->bufferInfo.format;
	if (bufferViewInfo->newFormat != rhi::Format::UNDEFINED)
		viewFormat = bufferViewInfo->newFormat;

	if (rhi::Utils::is_format_srgb(viewFormat))
		viewFormat = rhi::Utils::get_non_srgb_format(viewFormat);

	if (viewFormat == rhi::Format::UNDEFINED)
	{
		create(buffer, bufferViewInfo->size, bufferViewInfo->offset);
	}
	else
	{
		if (buffer->bufferInfo.size < bufferViewInfo->offset + bufferViewInfo->size)
			LOG_FATAL("VulkanBufferView::create(): Buffer view needs more memory than buffer can provide")

		if (!has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::STORAGE_TEXEL_BUFFER) ||
			!has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::UNIFORM_TEXEL_BUFFER))
		{
			LOG_FATAL("VulkanBufferView::create(): Buffer usage is neither STORAGE_TEXEL_BUFFER nor UNIFORM_TEXEL_BUFFER")
		}

		VkBufferViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		createInfo.buffer = get_vk_obj(buffer)->get_handle();
		createInfo.offset = bufferViewInfo->offset;
		createInfo.range = bufferViewInfo->size;
		createInfo.format = get_format(viewFormat);
		vkCreateBufferView(device->get_device(), &createInfo, nullptr, &std::get<VkBufferView>(_bufferView));
	}
}

void VulkanBufferView::create(rhi::Buffer* buffer, uint64_t size, uint64_t offset)
{
	if (!is_raw())
		LOG_FATAL("VulkanBufferView::create(): Can't change buffer view type from typed buffer to raw buffer")
	
	_bufferView = VkDescriptorBufferInfo{ get_vk_obj(buffer)->get_handle(), offset, size };
}

void VulkanBufferView::destroy(VulkanDevice* device)
{
	if (!is_raw())
	{
		if (get_handle() != VK_NULL_HANDLE)
		{
			vkDestroyBufferView(device->get_device(), get_handle(), nullptr);
		}
	}
}
