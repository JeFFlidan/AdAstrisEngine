#include "model_importer.h"
#include "gltf_importer.h"
#include "obj_importer.h"
#include "fbx_importer.h"

using namespace ad_astris;
using namespace resource::impl;

ModelImportContext::ModelImportContext(void* conversionContext)
{
	this->conversionContext = static_cast<ecore::ModelConversionContext*>(conversionContext);
}

bool ModelImporter::import(const io::URI& path, ModelImportContext& context)
{
	const std::string extension = io::Utils::get_file_extension(path);
	if (extension == "gltf" || extension == "glb")
	{
		if (!GLTFImporter::import(
			path.c_str(),
			context.modelCreateInfos,
			context.textureCreateInfos,
			context.materialCreateInfos,
			*context.conversionContext))
		{
			LOG_ERROR("ResourceManager::convert_to_engine_format(): Failed to import 3D model {}", path.c_str())
			return false;
		}
	}
	else if (extension == "obj")
	{
		if (!OBJImporter::import(
			path.c_str(),
			context.modelCreateInfos,
			context.textureCreateInfos,
			context.materialCreateInfos,
			*context.conversionContext))
		{
			LOG_ERROR("ResourceManager::convert_to_engine_format(): Failed to import 3D model {}", path.c_str())
			return false;
		}
	}
	else if (extension == "fbx")
	{
		if (!FBXImporter::import(
			path.c_str(),
			context.modelCreateInfos,
			context.textureCreateInfos,
			context.materialCreateInfos,
			*context.conversionContext))
		{
			LOG_ERROR("ResourceManager::convert_to_engine_format(): Failed to import 3D model {}", path.c_str())
			return false;
		}
	}
	return true;
}
