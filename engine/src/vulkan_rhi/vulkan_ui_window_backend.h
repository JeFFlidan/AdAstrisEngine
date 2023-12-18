#pragma once

#include "api.h"
#include "vulkan_rhi.h"
#include "rhi/ui_window_backend.h"
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>

namespace ad_astris::vulkan
{
	class VK_RHI_API VulkanUIWindowBackend : public rhi::UIWindowBackend
	{
		public:
			virtual void init(rhi::UIWindowBackendInitContext& initContext, rhi::Sampler sampler) override;
			virtual void cleanup() override;
			virtual void draw(rhi::CommandBuffer* cmd) override;
			virtual void resize(uint32_t width, uint32_t height) override;
			virtual void get_callbacks(rhi::UIWindowBackendCallbacks& callbacks) override;
			virtual void set_backbuffer(rhi::TextureView* textureView, rhi::Sampler sampler) override;

		private:
			VulkanRHI* _rhi{ nullptr };
			io::FileSystem* _fileSystem{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };
			VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
			VkDescriptorSet _descriptorSet{ VK_NULL_HANDLE };
			std::unordered_map<editor::IconType, uicore::TextureInfo> _icons;
			ImFont* _fontSize14;
			ImFont* _fontSize17;

			void create_icon(rhi::CommandBuffer& cmd, rhi::Buffer& cpuBuffer, rhi::Sampler& sampler, uicore::TextureInfo& iconInfo, unsigned char* imageData);
	};
}
