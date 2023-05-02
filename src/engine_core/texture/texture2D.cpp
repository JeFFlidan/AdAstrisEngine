#include "texture2D.h"
#include <utility>

using namespace ad_astris;

void ecore::Texture2D::serialize(io::IFile* file)
{
	std::string strMetadata = texture::Utils::pack_texture2D_info(&_textureInfo);
	file->set_metadata(strMetadata);
}

void ecore::Texture2D::deserialize(io::IFile* file, ObjectName* newName)
{
	_data = file->get_binary_blob();
	_textureInfo = texture::Utils::unpack_texture2D_info(file->get_metadata());
	_path = file->get_file_path();
	if (!newName)
	{
		_name = ObjectName(file->get_file_name().c_str());
	}
	else
	{
		_name = *newName;
	}
}

uint64_t ecore::Texture2D::get_size()
{
	return _textureInfo.size;
}

bool ecore::Texture2D::is_resource()
{
	return true;
}

UUID ecore::Texture2D::get_uuid()
{
	return _textureInfo.uuid;
}

std::string ecore::Texture2D::get_description()
{
	// TODO
}

void ecore::Texture2D::rename_in_engine(ObjectName& newName)
{
	// TODO
}

std::string ecore::Texture2D::get_type()
{
	return "texture";
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

