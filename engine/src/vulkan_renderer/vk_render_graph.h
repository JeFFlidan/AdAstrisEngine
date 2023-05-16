#pragma once

#include "vulkan/vulkan.h"

#include <string>
#inlcude <vector>

namespace engine::render_graph
{
	struct AttachmentInfo
	{
		float sizeX{ 1.0f };
		float sizeY{ 1.0f };
		VkFormat format{ VK_FORMAT_UNDEFINED };
		uint8_t samples{ 1 };
		uint8_t levels{ 1 };
		uint8_t layers{ 1 };
		bool persistent{ true };
	};

	struct BufferInfo
	{
		VkDeviceSize size{ 0 };
		VkBufferUsageFlags usage{ 0 };
		bool persistent{ true };
	};
}