#include "video.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace ecore;

Video::Video(const VideoInfo& info, ObjectName* name) : _info(info)
{
	_name = name;
}

void Video::serialize(io::File* file)
{
	LOG_WARNING("Video::serialize(): NOT IMPLEMENTED")
}

void Video::deserialize(io::File* file, ObjectName* objectName)
{
	LOG_WARNING("Video::deserialize(): NOT IMPLEMENTED")
}

void Video::accept(resource::IResourceVisitor& resourceVisitor)
{
	
}

