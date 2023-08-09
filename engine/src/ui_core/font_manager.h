#pragma once

#include <imgui.h>
#include <string>
#include <unordered_map>

namespace ad_astris::uicore
{
	class FontManager
	{
		public:
			static void load_default_fonts(const std::string& rootPath);
			static ImFont* get_default_font_size_17();
			static ImFont* get_default_font_size_14();
			static ImFont* load_custom_font(const std::string& fontAbsolutePath, float fontSize);
			static ImFont* get_custom_font(const std::string& fontName);
			static float get_custom_font_size(const std::string& fontName);

		private:
			struct FontInfo
			{
				ImFont* fontHandle;
				float fontSize;
			};
			static FontInfo _defaultFontSize17Info;
			static FontInfo _defaultFontSize14Info; 
			static std::unordered_map<std::string, FontInfo> _customFontInfos;
	};
}
