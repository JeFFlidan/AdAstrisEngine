#pragma once

#include "resources.h"

namespace ad_astris::rhi
{
	class IEngineRHI
	{
		public:
			virtual ~IEngineRHI() = default;
		
			virtual void init() = 0;
			virtual void cleanup() = 0;

			virtual void create_buffer(BufferInfo* info, void* data = nullptr, uint64_t size = 0) = 0;
			virtual void create_texture(TextureInfo* info, void* data = nullptr, uint64_t size = 0) = 0;
			virtual void create_sampler(SamplerInfo* info) = 0;

			virtual void update_buffer_data(BufferInfo* info, void* data, uint64_t size) = 0;
	};
}
