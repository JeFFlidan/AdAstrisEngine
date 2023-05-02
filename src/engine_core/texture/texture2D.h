#pragma once

#include "texture_common.h"
#include "file_system/file.h"
#include "engine_core/object.h"

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
		
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* newName = nullptr) override;
			virtual uint64_t get_size() override;
			virtual bool is_resource() override;
			virtual UUID get_uuid() override;
			virtual std::string get_description() override;
			virtual std::string get_type() override;
		
		protected:
			virtual void rename_in_engine(ObjectName& newName) override;

			// ========== End Object interface ==========

		private:
			texture::Texture2DInfo _textureInfo;
			uint8_t* _data;
	};
}

namespace ad_astris::io
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
		
		void get_data(std::string& metadata,uint8_t*& binBlob,uint64_t& binBlobSize,URI& path);
	};
}