#pragma once

#include "internal/base_widget.h"
#include <imgui/imgui.h>
#include <cstdint>
#include <functional>

namespace ad_astris::uicore
{
	struct TextureInfo
	{
		int32_t width;
		int32_t height;
		uint32_t textureID;
		ImVec2 uv0 = { 0, 0 };
		ImVec2 uv1 = { 1, 1 };
	};

	using UICallback = std::function<void(internal::BaseWidget&)>;
	using UISetCustomStyleCallback = std::function<void()>;

	class IUIWindow
	{
		public:
			virtual ~IUIWindow() {}
			virtual void draw_window(void* data = nullptr) = 0;
	};
}