#include "renderer_subsettings.h"
#include "core/reflection.h"

using namespace ad_astris;
using namespace ecore;

void RendererSubsettings::serialize(Config& config)
{
	Section section(get_type_name<RendererSubsettings>());
	serialize_graphics_api(section);

	config.set_section(section);
}

void RendererSubsettings::deserialize(Section& section)
{
	deserialize_graphics_api(section);
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
