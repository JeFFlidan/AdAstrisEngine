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
	LOG_WARNING("Font::serialize(): NOT IMPLEMENTED")
}

void Font::deserialize(io::File* file, ObjectName* objectName)
{
	LOG_WARNING("Font::deserialize(): NOT IMPLEMENTED")
}

void Font::accept(resource::IResourceVisitor& resourceVisitor)
{
	
}
