#pragma once

#include "engine_rhi.h"
#include "application_core/window.h"
#include "ui_core/common.h"
#include <imgui/imgui.h>
#include <functional>

namespace ad_astris::editor
{
	enum class IconType
	{
		FOLDER,
		TEXTURE_FILE,
		MODEL_FILE,
		LEVEL_FILE,
		MATERIAL_FILE
	};
}

namespace ad_astris::rhi
{
	using GetIconsCallback = std::function<std::unordered_map<editor::IconType, uicore::TextureInfo>()>;
	
	struct UIWindowBackendCallbacks
	{
		struct ImGuiAllocators
		{
			ImGuiMemAllocFunc allocFunc;
			ImGuiMemFreeFunc freeFunc;
		};
		std::function<void()> beginFrameCallback;
		std::function<ImGuiContext*()> getContextCallback;
		std::function<ImGuiAllocators()> getImGuiAllocators;
		std::function<uint64_t()> getViewportImageCallback;
		std::function<ImFont*()> getDefaultFont14;
		std::function<ImFont*()> getDefaultFont17;
		GetIconsCallback getContentIcons;
	};

	struct UIWindowBackendInitContext
	{
		IEngineRHI* rhi;
		acore::IWindow* window;
		io::FileSystem* fileSystem;
		Sampler sampler;
		RenderPass renderPass;
	};
	
	// ImGui backend for one window instance (for example, WinApi window).
	// I'm not sure that I will use multiple window instances, however I think it will be good to have this possibility
	class UIWindowBackend
	{
		public:
			virtual void init(UIWindowBackendInitContext& initContext, Sampler sampler) = 0;
			virtual void cleanup() = 0;
			virtual void draw(CommandBuffer* cmd) = 0;
			virtual void resize(uint32_t width, uint32_t height) = 0;
			virtual void get_callbacks(UIWindowBackendCallbacks& callbacks) = 0;
			virtual void set_backbuffer(TextureView* textureView, Sampler sampler) = 0;
	};
}
