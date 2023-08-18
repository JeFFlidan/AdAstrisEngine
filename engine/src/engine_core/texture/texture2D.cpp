#include "texture2D.h"
#include <utility>

#include "resource_manager/resource_visitor.h"

using namespace ad_astris;

void ecore::Texture2D::serialize(io::IFile* file)
{
	std::string strMetadata = texture::Utils::pack_texture2D_info(&_textureInfo);
	file->set_metadata(strMetadata);
}

void ecore::Texture2D::deserialize(io::IFile* file, ObjectName* objectName)
{
	_data = file->get_binary_blob();
	_textureInfo = texture::Utils::unpack_texture2D_info(file->get_metadata());
	_path = file->get_file_path();
	_name = objectName;
}

inline uint64_t ecore::Texture2D::get_size()
{
	return _textureInfo.size;
}

inline bool ecore::Texture2D::is_resource()
{
	return true;
}

inline UUID ecore::Texture2D::get_uuid()
{
	return _textureInfo.uuid;
}

inline std::string ecore::Texture2D::get_description()
{
	// TODO
}

inline std::string ecore::Texture2D::get_type()
{
	return "texture";
}

inline void ecore::Texture2D::accept(resource::IResourceVisitor& resourceVisitor)
{
	resourceVisitor.visit(this);
}

void io::ConversionContext<ecore::Texture2D>::get_data(std::string& metadata,uint8_t*& binBlob,uint64_t& binBlobSize,URI& path)
{
	ecore::texture::Texture2DInfo info;
	info.uuid = uuid;
	info.size = size;
	info.width = width;
	info.height = height;
	info.originalFile = originalFile;
	
	info.brightness = oldInfo ? oldInfo->brightness : 1.0;
	info.saturation = oldInfo ? oldInfo->saturation : 1.0;
	info.mipmapMode = oldInfo ? oldInfo->mipmapMode : ecore::texture::MipmapMode::BASE_MIPMAPPING;
	info.tilingX = oldInfo ? oldInfo->tilingX : ecore::texture::TilingMode::REPEAT;
	info.tilingY = oldInfo ? oldInfo->tilingY : ecore::texture::TilingMode::REPEAT;
	info.sRGB = oldInfo ? oldInfo->sRGB : true;
	info.runtimeCompressionMode = oldInfo ? oldInfo->runtimeCompressionMode : ecore::texture::RuntimeCompressionMode::DXT1; // Have to think
	
	metadata = ecore::texture::Utils::pack_texture2D_info(&info);
	
	binBlobSize = size;
	path = filePath.c_str();
	binBlob = pixels;

	if (oldInfo)
		delete oldInfo;
}

