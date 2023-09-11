#pragma once

#include <cstdint>

namespace ad_astris::vulkan
{
	constexpr uint32_t UNDEFINED_DESCRIPTOR = ~0u;
	
	class VulkanDescriptor
	{
		public:
			uint32_t get_descriptor_index() { return _descriptorIndex; }
			void set_descriptor_index(uint32_t index) { _descriptorIndex = index; }

		private:
			uint32_t _descriptorIndex{ UNDEFINED_DESCRIPTOR };
	};
}