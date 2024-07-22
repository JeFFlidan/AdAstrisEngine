#pragma once

namespace ad_astris::renderer
{
	struct AttachmentName
	{
		constexpr static const char* TAA_OUTPUT = "TAAOutput";
		constexpr static const char* OIT_COLOR = "OITColor";
		constexpr static const char* OIT_VELOCITY = "OITVelocity";
		constexpr static const char* DEFERRED_LIGHTING_OUTPUT = "DeferredLightingOutput";
		constexpr static const char* G_ALBEDO = "gAlbedo";
		constexpr static const char* G_NORMAL = "gNormal";
		constexpr static const char* G_SURFACE = "gSurface";
		constexpr static const char* G_DEPTH_STENCIL = "gDepthStencil";
	};
}