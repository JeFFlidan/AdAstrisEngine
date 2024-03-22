#include "font.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace ecore;

Font::Font(const FontInfo& info, ObjectName* name) : _info(info)
{
	_name = name;
}

void Font::serialize(io::File* file)
{
	uint8_t* blob = new uint8_t[_info.blob.size()];
	memcpy(blob, _info.blob.data(), _info.blob.size());
	file->set_binary_blob(blob, _info.blob.size());
}

void Font::deserialize(io::File* file, ObjectName* objectName)
{
	_name = objectName;
	_path = file->get_file_path();
	_info.init(file->get_binary_blob(), file->get_binary_blob_size());
}

void Font::accept(resource::IResourceVisitor& resourceVisitor)
{
	
}

void FontInfo::init()
{
	int offset = stbtt_GetFontOffsetForIndex(blob.data(), 0);
	if (!stbtt_InitFont(&info, blob.data(), offset))
	{
		LOG_ERROR("FontInfo::init(): Failed to init font")
		return;
	}

	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
}

void FontInfo::init(const uint8_t* fontBlob, size_t fontBlobSize)
{
	blob.resize(fontBlobSize);
	memcpy(blob.data(), fontBlob, fontBlobSize);
	init();
}
