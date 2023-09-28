#include "renderer_subsettings.h"
#include "core/reflection.h"

using namespace ad_astris;
using namespace ecore;

void RendererSubsettings::serialize(Config& config)
{
	Section section(get_type_name<RendererSubsettings>());
	serialize_graphics_api(section);
	serialize_swap_chain_desc(section);
	section.set_option("DefaultMaterialUUID", (uint64_t)_defaultMaterialUUID);

	config.set_section(section);
}

void RendererSubsettings::deserialize(Section& section)
{
	deserialize_graphics_api(section);
	deserialize_swap_chain_desc(section);
	_defaultMaterialUUID= section.get_option_value<uint64_t>("DefaultMaterialUUID");
}

void RendererSubsettings::setup_default_values()
{
	_graphicsAPI = GraphicsAPI::VULKAN;
	_swapChainDesc.useTripleBuffering = false;
	_swapChainDesc.useVSync = true;
	_swapChainDesc.width = 1280;
	_swapChainDesc.height = 720;
}

void RendererSubsettings::serialize_graphics_api(Section& section)
{
	switch (_graphicsAPI)
	{
		case GraphicsAPI::VULKAN:
			section.set_option("GraphicsAPI", "Vulkan");
		break;
		case GraphicsAPI::D3D12:
			section.set_option( "GraphicsAPI", "D3D12");
		break;
		case GraphicsAPI::UNDEFINED:
			LOG_FATAL("EngineSettings::serialize(): Graphics API can't be undefined")
			break;
	}
}

void RendererSubsettings::deserialize_graphics_api(Section& section)
{
	std::string strGraphicsAPI = section.get_option_value<std::string>("GraphicsAPI");

	if (strGraphicsAPI == "Vulkan")
		_graphicsAPI = GraphicsAPI::VULKAN;
	if (strGraphicsAPI == "D3D12")
		_graphicsAPI = GraphicsAPI::D3D12;
}

void RendererSubsettings::serialize_swap_chain_desc(Section& section)
{
	section.set_option("RenderAreaWidth", _swapChainDesc.width);
	section.set_option("RenderAreaHeight", _swapChainDesc.height);
	section.set_option("UseTripleBuffering", _swapChainDesc.useTripleBuffering);
	section.set_option("UseVSync", _swapChainDesc.useVSync);
}

void RendererSubsettings::deserialize_swap_chain_desc(Section& section)
{
	_swapChainDesc.width = section.get_option_value<uint64_t>("RenderAreaWidth");
	_swapChainDesc.height = section.get_option_value<uint64_t>("RenderAreaHeight");
	_swapChainDesc.useTripleBuffering = section.get_option_value<bool>("UseTripleBuffering");
	_swapChainDesc.useVSync = section.get_option_value<bool>("UseVSync");
}
