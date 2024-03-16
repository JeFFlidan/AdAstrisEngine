#include "sound.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace ecore;

Sound::Sound(const SoundInfo& info, ObjectName* name)
{
	
}

void Sound::serialize(io::File* file)
{
	LOG_WARNING("Sound::serialize(): NOT IMPLEMENTED")
}

void Sound::deserialize(io::File* file, ObjectName* objectName)
{
	LOG_WARNING("Sound::deserialize(): NOT IMPLEMENTED")
}

void Sound::accept(resource::IResourceVisitor& resourceVisitor)
{
	
}
