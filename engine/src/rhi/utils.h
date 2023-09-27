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
	};
}