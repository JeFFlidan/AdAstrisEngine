#pragma once

#include "file_system/file_system.h"
#include "resource_formats.h"

#include <json.hpp>

namespace ad_astris::resource
{
	class ResourceConverter
	{
		public:
			ResourceConverter(io::FileSystem* fileSystem);

			/** Creates an aares file for later use in the engine. Can be used for models and textures
			 * @param path must be a valid path to a 3D object file (glTF, obj, fbx, etc)
			 * or texture file (tga, png, tiff, etc.)
			 * @param existedInfo must be a valid pointer to the existed model info or texture info if
			 * resource is going to be reloaded. Otherwise, must be nullptr
			*/
			void* convert_to_aares_file(io::URI& path, void* existedInfo = nullptr);
		private:
			io::FileSystem* _fileSystem{ nullptr };

			void write_info_to_disk(ModelInfo* modelInfo);
			void write_info_to_disk(TextureInfo* textureInfo);
			void convert_to_model_info_from_gltf(io::URI& path, ModelInfo* modelInfo);
			void convert_to_model_info_from_obj(io::URI& path, ModelInfo* modelInfo);
			void convert_to_texture_info_from_raw_image(io::URI& path, TextureInfo* texInfo);

			struct BinaryBlob
			{
				BinaryBlob(uint64_t size, void* binBlob) : size(size), binaryBlob(binBlob) {}
				uint64_t size;
				void* binaryBlob;
			};

			void write_to_disk(io::URI& path, ResourceInfo& resourceInfo);
	};
}