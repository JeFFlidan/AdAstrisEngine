#include "font_manager.h"
#include "file_system/utils.h"
#include "profiler/logger.h"

using namespace ad_astris::uicore;

FontManager::FontInfo FontManager::_defaultFontSize14Info;
FontManager::FontInfo FontManager::_defaultFontSize17Info;
std::unordered_map<std::string, FontManager::FontInfo> FontManager::_customFontInfos;

ImFont* FontManager::get_default_font_size_14()
{
	return _defaultFontSize14Info.fontHandle;
}

ImFont* FontManager::get_default_font_size_17()
{
	return _defaultFontSize17Info.fontHandle;
}

void FontManager::load_default_fonts(const std::string& rootPath)
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	_defaultFontSize17Info.fontHandle = io.Fonts->AddFontFromFileTTF((rootPath + "/fonts/unispace bd.ttf").c_str(), 17.0f);
	_defaultFontSize17Info.fontSize = 17.0f;
	IM_ASSERT(_defaultFontSize17Info.fontHandle != nullptr);

	_defaultFontSize14Info.fontHandle = io.Fonts->AddFontFromFileTTF((rootPath + "/fonts/unispace bd.ttf").c_str(), 14.0f);
	_defaultFontSize14Info.fontSize = 14.0f;
	IM_ASSERT(_defaultFontSize14Info.fontHandle != nullptr);
}

ImFont* FontManager::load_custom_font(const std::string& fontAbsolutePath, float fontSize)
{
	ImGuiIO& io = ImGui::GetIO();
	FontInfo fontInfo;
	fontInfo.fontHandle = io.Fonts->AddFontFromFileTTF(fontAbsolutePath.c_str(), fontSize);
	fontInfo.fontSize = fontSize;
	IM_ASSERT(fontInfo.fontHandle != nullptr);
	std::string fontName = io::Utils::get_file_name(fontAbsolutePath.c_str());
	_customFontInfos[fontName] = fontInfo;
	return _customFontInfos[fontName].fontHandle;
}

ImFont* FontManager::get_custom_font(const std::string& fontName)
{
	auto it = _customFontInfos.find(fontName);
	if (it == _customFontInfos.end())
	{
		LOG_ERROR("FontManager::get_custom_font(): Font {} wasn't loaded")
		return nullptr;
	}
	return it->second.fontHandle;
}

float FontManager::get_custom_font_size(const std::string& fontName)
{
	auto it = _customFontInfos.find(fontName);
	if (it == _customFontInfos.end())
	{
		LOG_ERROR("FontManager::get_custom_font(): Font {} wasn't loaded")
		return 0.0f;
	}

	return it->second.fontSize;
}
