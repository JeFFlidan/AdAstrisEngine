#pragma once

#include "profiler_instance.h"

namespace ad_astris::profiler
{
	class Serializer
	{
		public:
			static void init(const ProfilerInstance* profilerInstance, io::FileSystem* fileSystem);
			static void serialize(const std::string& statisticsFileName);
			//static void deserialize();

		private:
			inline static const ProfilerInstance* _profilerInstance{ nullptr };
			inline static io::FileSystem* _fileSystem{ nullptr };

			static bool is_initialized();
	};
}
