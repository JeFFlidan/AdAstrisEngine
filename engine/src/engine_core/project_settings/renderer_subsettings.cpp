#include "renderer_subsettings.h"
#include "core/reflection.h"

using namespace ad_astris;
using namespace ecore;

void RendererSubsettings::serialize(Config& config)
{
	Section section(get_type_name<RendererSubsettings>());
	serialize_graphics_api(section);
	serialize_swap_chain_desc(section);
	serialize_scene_culling_settings(section);
	serialize_shadows_culling_settings(section);
	section.set_option("DefaultMaterialUUID", (uint64_t)_defaultMaterialUUID);

	config.set_section(section);
}

void RendererSubsettings::deserialize(Section& section)
{
	deserialize_graphics_api(section);
	deserialize_swap_chain_desc(section);
	deserialize_scene_culling_settings(section);
	deserialize_shadows_culling_settings(section);
	_defaultMaterialUUID= section.get_option_value<uint64_t>("DefaultMaterialUUID");
}

void RendererSubsettings::setup_default_values()
{
	_graphicsAPI = GraphicsAPI::VULKAN;
	_swapChainDesc.useTripleBuffering = false;
	_swapChainDesc.useVSync = true;
	_swapChainDesc.width = 1280;
	_swapChainDesc.height = 720;
	_sceneCullingSettings.drawDistance = 999999;
	_sceneCullingSettings.isFrustumCullingEnabled = true;
	_sceneCullingSettings.isOcclusionCullingEnabled = true;
	_sceneCullingSettings.lodBase = 2.0f;	// TODO read about lod
	_sceneCullingSettings.lodStep = 1.0f;
	_shadowsCulllingSettings.drawDistance = 999999;
	_shadowsCulllingSettings.isFrustumCullingEnabled = true;
	_shadowsCulllingSettings.isOcclusionCullingEnabled = false;
	_shadowsCulllingSettings.isAabbCheckEnabled = true;
	_shadowsCulllingSettings.lodBase = 2.0f;
	_shadowsCulllingSettings.lodStep = 1.0f;
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

void RendererSubsettings::serialize_scene_culling_settings(Section& section)
{
	section.set_option("SceneDrawDistance", _sceneCullingSettings.drawDistance);
	section.set_option("SceneIsFrustumCullingEnabled", _sceneCullingSettings.isFrustumCullingEnabled);
	section.set_option("SceneIsOcclusionCullingEnabled", _sceneCullingSettings.isOcclusionCullingEnabled);
	section.set_option("SceneLodBase", _sceneCullingSettings.lodBase);
	section.set_option("SceneLodStep", _sceneCullingSettings.lodStep);
}

void RendererSubsettings::deserialize_scene_culling_settings(Section& section)
{
	_sceneCullingSettings.drawDistance = section.get_option_value<uint64_t>("SceneDrawDistance");
	_sceneCullingSettings.isFrustumCullingEnabled = section.get_option_value<bool>("SceneIsFrustumCullingEnabled");
	_sceneCullingSettings.isOcclusionCullingEnabled = section.get_option_value<bool>("SceneIsOcclusionCullingEnabled");
	_sceneCullingSettings.lodBase = section.get_option_value<double>("SceneLodBase");
	_sceneCullingSettings.lodStep = section.get_option_value<double>("SceneLodStep");
}

void RendererSubsettings::serialize_shadows_culling_settings(Section& section)
{
	section.set_option("ShadowsDrawDistance", _shadowsCulllingSettings.drawDistance);
	section.set_option("ShadowsIsFrustumCullingEnabled", _shadowsCulllingSettings.isFrustumCullingEnabled);
	section.set_option("ShadowsIsOcclusionCullingEnabled", _shadowsCulllingSettings.isOcclusionCullingEnabled);
	section.set_option("ShadowsIsAabbCheckEnabled", _shadowsCulllingSettings.isAabbCheckEnabled);
	section.set_option("ShadowsLodBase", _shadowsCulllingSettings.lodBase);
	section.set_option("ShadowsLodStep", _shadowsCulllingSettings.lodStep);
}

void RendererSubsettings::deserialize_shadows_culling_settings(Section& section)
{
	_shadowsCulllingSettings.drawDistance = section.get_option_value<uint64_t>("ShadowsDrawDistance");
	_shadowsCulllingSettings.isFrustumCullingEnabled = section.get_option_value<bool>("ShadowsIsFrustumCullingEnabled");
	_shadowsCulllingSettings.isOcclusionCullingEnabled = section.get_option_value<bool>("ShadowsIsOcclusionCullingEnabled");
	_shadowsCulllingSettings.isAabbCheckEnabled = section.get_option_value<bool>("ShadowsIsAabbCheckEnabled");
	_shadowsCulllingSettings.lodBase = section.get_option_value<double>("ShadowsLodBase");
	_shadowsCulllingSettings.lodStep = section.get_option_value<double>("ShadowsLodStep");
}
