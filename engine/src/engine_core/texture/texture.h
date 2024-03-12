#pragma once

#include "texture_common.h"
#include "engine_core/object.h"
#include "rhi/resources.h"

namespace ad_astris::ecore
{
	struct TextureInfo
	{
		uint8_t* data{ nullptr };
		uint64_t size{ 0 };		// Texture size without mipmaps
		uint64_t width{ 0 };
		uint64_t height{ 0 };
		uint64_t depth{ 0 };
		texture::MipmapMode mipmapMode{ texture::MipmapMode::BASE_MIPMAPPING };
		texture::RuntimeCompressionMode runtimeCompressionMode{ texture::RuntimeCompressionMode::DISABLED };
		rhi::AddressMode tilingX{ rhi::AddressMode::REPEAT };
		rhi::AddressMode tilingY{ rhi::AddressMode::REPEAT };
		rhi::Format format{ rhi::Format::UNDEFINED };
		rhi::ComponentMapping mapping;
		float brightness{ 1.0f };
		float saturation{ 1.0f };
	};
	
	class Texture : public Object
	{
		public:
			Texture() = default;
			Texture(const TextureInfo& textureInfo);
			~Texture() override;

			TextureInfo& get_texture_info() { return _textureInfo; }
			const TextureInfo& get_texture_info() const { return _textureInfo; }

			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			void accept(resource::IResourceVisitor& resourceVisitor) override;
			uint64_t get_size() override { return _textureInfo.size; }
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Texture"; }
			std::string get_type() override { return "texture"; }

			void update_texture(uint8_t* textureData, uint64_t sizeInBytes);
			void destroy_texture_data();
		
		private:
			TextureInfo _textureInfo;
			UUID _uuid;
	};
}