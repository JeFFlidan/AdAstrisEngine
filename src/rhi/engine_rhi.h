#pragma once

#include "resources.h"

namespace ad_astris::rhi
{
	class IEngineRHI
	{
		public:
			virtual ~IEngineRHI() = default;
		
			virtual void init(void* window) = 0;
			virtual void cleanup() = 0;

			virtual void create_buffer(Buffer* buffer, uint64_t size, void* data = nullptr) = 0;
			virtual void update_buffer_data(Buffer* buffer, uint64_t size, void* data) = 0;
			virtual void create_texture(Texture* texture) = 0;
			virtual void create_texture_view(TextureView* textureView, Texture* texture) = 0;
			virtual void create_sampler(Sampler* sampler) = 0;
	};
}
