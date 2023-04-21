#pragma once

#include "file_system/file_system.h"
#include "resource_formats.h"

namespace ad_astris::resource
{
	class ResourceConverter
	{
		public:
			ResourceConverter() = default;
			ResourceConverter(io::FileSystem* fileSystem);

			/** Creates an aares file for later use in the engine
			 * @param path must be a valid path to a 3D object file (glTF, obj, fbx, etc)
			 * or texture file (tga, png, tiff, etc.)
			 * @param existedInfo must be a valid pointer to the existed resource info if
			 * resource is going to be reloaded. Otherwise, must be nullptr
			*/
			ResourceInfo convert_to_aares_file(io::URI& path, ResourceInfo* existedInfo = nullptr);
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
		
			void convert_to_model_info_from_gltf(io::URI& path, ModelInfo* modelInfo, ResourceInfo* resourceInfo);
			void convert_to_model_info_from_obj(io::URI& path, ModelInfo* modelInfo, ResourceInfo* resourceInfo);
			void convert_to_texture_info_from_raw_image(io::URI& path, TextureInfo* texInfo, ResourceInfo* resourceInfo);

			struct BinaryBlob
			{
				BinaryBlob(uint64_t size, void* binBlob) : size(size), binaryBlob(binBlob) {}
				uint64_t size;
				void* binaryBlob;
			};
	};
}