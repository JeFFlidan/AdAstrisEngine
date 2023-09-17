#pragma once

#include "texture_common.h"
#include "file_system/file.h"
#include "engine_core/object.h"
#include "resource_manager/resource_files.h"

namespace ad_astris::ecore
{
	class Texture2D : public Object
	{
		friend texture::Utils;

		public:
			Texture2D() = default;

			// Temporary solution. Have to figure out how to work with texture metadata
			texture::Texture2DInfo get_info()
			{
				return _textureInfo;
			}
		public:
			// ========== Begin Object interface ==========
		
			virtual void serialize(io::File* file) override;
			virtual void deserialize(io::File* file, ObjectName* objectName) override;
			virtual uint64_t get_size() override;
			virtual bool is_resource() override;
			virtual UUID get_uuid() override;
			virtual std::string get_description() override;
			virtual std::string get_type() override;
			virtual void accept(resource::IResourceVisitor& resourceVisitor) override;

			// ========== End Object interface ==========

		private:
			texture::Texture2DInfo _textureInfo;
			uint8_t* _data;
	};
}

namespace ad_astris::resource
{
	template<>
	struct ConversionContext<ecore::Texture2D>
	{
		UUID uuid;
		uint8_t* pixels;
		uint64_t size;
		uint64_t width;
		uint64_t height;
		std::string originalFile;
		std::string filePath;
		ecore::texture::Texture2DInfo* oldInfo{ nullptr };
		
		void get_data(std::string& metadata,uint8_t*& binBlob,uint64_t& binBlobSize,io::URI& path);
	};
}