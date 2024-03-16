#include "fbx_importer.h"

using namespace ad_astris;
using namespace resource::impl;

bool FBXImporter::import(
	const std::string& path,
	std::vector<ecore::ModelInfo>& outModelInfos,
	std::vector<ecore::TextureInfo>& outTextureInfos,
	std::vector<ecore::MaterialInfo>& materialInfos,
	const ecore::ModelConversionContext& conversionContext)
{
	return true;
}

