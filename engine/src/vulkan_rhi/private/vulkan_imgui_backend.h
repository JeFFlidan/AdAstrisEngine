#pragma once

#include "vulkan_rhi.h"
#include "rhi/imgui_backend.h"
#include <unordered_map>

namespace ad_astris::vulkan
{
	class VulkanImGuiBackend : public rhi::IImGuiBackend
	{
		public:
			void init(rhi::ImGuiBackendInitContext& initContext) override;
			void cleanup() override;
			void begin_frame() const override;
			void draw(rhi::CommandBuffer* cmd) override;
			void set_backbuffer(rhi::TextureView* textureView, rhi::Sampler sampler) override;
			void load_texture(rhi::CommandBuffer* cmd, const io::URI& texturePath) override;
			const uicore::TextureInfo& get_texture_info(const std::string& textureName) const override;
		
			ImFont* get_default_font_14() const override { return _defaultFont14; }
			ImFont* get_default_font_17() const override { return _defaultFont17; }
			uint64_t get_viewport_image_id() const override { return (uint64_t)_backbufferDescriptorSet; }
			ImGuiContext* get_context() const override { return _imguiContext; }
			ImGuiMemAllocFunc get_alloc_func() const override { return _imguiAllocFunc; }
			ImGuiMemFreeFunc get_free_func() const override { return _imguiFreeFunc; }

		private:
			VulkanRHI* _rhi{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };
			rhi::Sampler _sampler;
			VkDescriptorPool _descriptorPool{ VK_NULL_HANDLE };
			VkDescriptorSet _backbufferDescriptorSet{ VK_NULL_HANDLE };
			std::unordered_map<std::string, uicore::TextureInfo> _textureByName;
			ImFont* _defaultFont14{ nullptr };
			ImFont* _defaultFont17{ nullptr };
			std::vector<rhi::Buffer> _cpuBuffers;

			ImGuiMemAllocFunc _imguiAllocFunc{ nullptr };
			ImGuiMemFreeFunc _imguiFreeFunc{ nullptr };
			ImGuiContext* _imguiContext{ nullptr };
	};
}