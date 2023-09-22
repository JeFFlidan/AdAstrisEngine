#pragma once

#include "engine_rhi.h"
#include "application_core/window.h"
#include <imgui/imgui.h>
#include <functional>

namespace ad_astris::rhi
{
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
	};

	struct UIWindowBackendInitContext
	{
		IEngineRHI* rhi;
		acore::IWindow* window;
		Sampler sampler;
		RenderPass renderPass;
	};
	
	// ImGui backend for one window instance (for example, WinApi window).
	// I'm not sure that I will use multiple window instances, however I think it will be good to have this possibility
	class UIWindowBackend
	{
		public:
			virtual void init(UIWindowBackendInitContext& initContext) = 0;
			virtual void cleanup() = 0;
			virtual void draw(CommandBuffer* cmd) = 0;
			virtual void resize(uint32_t width, uint32_t height) = 0;
			virtual void get_callbacks(UIWindowBackendCallbacks& callbacks) = 0;
			virtual void set_backbuffer(TextureView* textureView, Sampler* sampler) = 0;
	};
}
