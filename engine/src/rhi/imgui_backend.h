#pragma once

#include "engine_rhi.h"
#include "resources.h"
#include "ui_core/common.h"
#include "application_core/window.h"

namespace ad_astris::rhi
{
	constexpr const char* FOLDER_ICON_NAME = "folder";
	constexpr const char* LEVEL_ICON_NAME = "level";
	constexpr const char* MODEL_ICON_NAME = "3Dmodel";
	constexpr const char* TEXTURE_ICON_NAME = "image";
	constexpr const char* MATERIAL_ICON_NAME = "material";
	
	struct ImGuiBackendInitContext
	{
		RHI* rhi{ nullptr };
		Sampler sampler;
		acore::IWindow* window{ nullptr };
	};
	
	class IImGuiBackend
	{
		public:
			virtual ~IImGuiBackend() = default;
		
			virtual void init(ImGuiBackendInitContext& initContext) = 0;
			virtual void cleanup() = 0;
			virtual void begin_frame() const = 0;
			virtual void draw(CommandBuffer* cmd) = 0;
			virtual void set_backbuffer(TextureView* textureView, Sampler sampler) = 0;
			virtual void load_texture(CommandBuffer* cmd, const io::URI& texturePath) = 0;
			virtual const uicore::TextureInfo& get_texture_info(const std::string& textureName) const = 0;
			virtual ImFont* get_default_font_14() const = 0;
			virtual ImFont* get_default_font_17() const = 0;
			virtual uint64_t get_viewport_image_id() const = 0;
			virtual ImGuiContext* get_context() const = 0;
			virtual ImGuiMemAllocFunc get_alloc_func() const = 0;
			virtual ImGuiMemFreeFunc get_free_func() const = 0;
	};

	inline void init_imgui(const IImGuiBackend* backend)
	{
		ImGui::SetCurrentContext(backend->get_context());
		ImGui::SetAllocatorFunctions(backend->get_alloc_func(), backend->get_free_func());
	}
}