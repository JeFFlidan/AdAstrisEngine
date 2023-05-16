#pragma once

#include "file_system/file_system.h"
#include "file_system/file.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"

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
			 * @param conversionContext contains all the necessary data for further use
			 * @param existedObject must be a valid pointer to an existing Object
			 * if the resource is going to be reloaded. Otherwise, must be nullptr
			*/
			template<typename T>
			void convert_to_aares_file(
				io::URI& path,
				io::ConversionContext<T>* conversionContext,
				ecore::Object* existedObject = nullptr);
		
		private:
			io::FileSystem* _fileSystem{ nullptr };
		
			void convert_to_model_info_from_gltf(io::URI& path, io::ConversionContext<ecore::StaticModel>* context);
			void convert_to_model_info_from_obj(io::URI& path, io::ConversionContext<ecore::StaticModel>* context);
			void convert_to_texture_info_from_raw_image(io::URI& path, io::ConversionContext<ecore::Texture2D>* context);

			struct BinaryBlob
			{
				BinaryBlob(uint64_t size, void* binBlob) : size(size), binaryBlob(binBlob) {}
				uint64_t size;
				void* binaryBlob;
			};
	};
}