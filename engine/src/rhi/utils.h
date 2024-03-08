#pragma once

#include "resources.h"

namespace ad_astris::rhi
{
	class Utils
	{
		public:
			static bool support_stencil(Format format)
			{
				if (format == Format::D32_SFLOAT || format == Format::D16_UNORM)
					return false;
				return true;
			}

			static uint32_t get_format_stride(Format format)
			{
				switch (format)
				{
					case Format::UNDEFINED:
						LOG_FATAL("rhi::Utils::get_format_stride(): Undefined format")
					case Format::R4G4_UNORM:
					case Format::R8_UNORM:
					case Format::R8_SNORM:
					case Format::R8_UINT:
					case Format::R8_SINT:
					case Format::R8_SRGB:
					case Format::S8_UINT:
						return 1u;
					case Format::R4G4B4A4_UNORM:
					case Format::B4G4R4A4_UNORM:
					case Format::R5G5B5A1_UNORM:
					case Format::B5G5R5A1_UNORM:
					case Format::A1R5G5B5_UNORM:
					case Format::R8G8_UNORM:
					case Format::R8G8_SNORM:
					case Format::R8G8_UINT:
					case Format::R8G8_SINT:
					case Format::R8G8_SRGB:
					case Format::R16_UNORM:
					case Format::R16_SNORM:
					case Format::R16_UINT:
					case Format::R16_SINT:
					case Format::R16_SFLOAT:
					case Format::D16_UNORM:
						return 2u;
					case Format::D16_UNORM_S8_UINT:
						return 3u;
					case Format::R8G8B8A8_UNORM:
					case Format::R8G8B8A8_SNORM:
					case Format::R8G8B8A8_UINT:
					case Format::R8G8B8A8_SINT:
					case Format::R8G8B8A8_SRGB:
					case Format::B8G8R8A8_SRGB:
					case Format::B8G8R8A8_UNORM:
					case Format::B8G8R8A8_SNORM:
					case Format::R10G10B10A2_UNORM:
					case Format::R10G10B10A2_SNORM:
					case Format::D24_UNORM_S8_UINT:
					case Format::D32_SFLOAT:
					case Format::R32_UINT:
					case Format::R32_SINT:
					case Format::R32_SFLOAT:
					case Format::R16G16_UNORM:
					case Format::R16G16_SNORM:
					case Format::R16G16_UINT:
					case Format::R16G16_SINT:
					case Format::R16G16_SFLOAT:
						return 4u;
					case Format::R16G16B16A16_UNORM:
					case Format::R16G16B16A16_SNORM:
					case Format::R16G16B16A16_UINT:
					case Format::R16G16B16A16_SINT:
					case Format::R16G16B16A16_SFLOAT:
					case Format::R32G32_UINT:
					case Format::R32G32_SINT:
					case Format::R32G32_SFLOAT:
					case Format::D32_SFLOAT_S8_UINT:
						return 8u;
					case Format::R32G32B32_UINT:
					case Format::R32G32B32_SINT:
					case Format::R32G32B32_SFLOAT:
						return 12u;
					case Format::R32G32B32A32_UINT:
					case Format::R32G32B32A32_SINT:
					case Format::R32G32B32A32_SFLOAT:
						return 16u;
					default:
						LOG_FATAL("rhi::Utils::get_format_stride(): Format does not have stride")
				}
			}

			static Format get_non_srgb_format(Format format)
			{
				switch (format)
				{
					case Format::R8_SRGB:
						return Format::R8_UNORM;
					case Format::R8G8_SRGB:
						return Format::R8G8_UNORM;
					case Format::R8G8B8A8_SRGB:
						return Format::R8G8B8A8_UNORM;
					case Format::B8G8R8A8_SRGB:
						return Format::B8G8R8A8_UNORM;
					default:
						return format;
				}
			}

			static bool is_format_srgb(Format format)
			{
				switch (format)
				{
					case Format::R8_SRGB:
					case Format::R8G8_SRGB:
					case Format::R8G8B8A8_SRGB:
					case Format::B8G8R8A8_SRGB:
						return true;
					default:
						return false;
				}
			}
	};
}