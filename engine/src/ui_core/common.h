#pragma once

#include <imgui.h>
#include <cstdint>

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
}