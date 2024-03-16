#pragma once

#include "engine_core/object.h"

namespace ad_astris::ecore
{
	struct SoundInfo
	{
		
	};
	
	class Sound : public Object
	{
		public:
			Sound() = default;
			Sound(const SoundInfo& info, ObjectName* name);

			void set_info(const SoundInfo& info) { _info = info; }
			const SoundInfo& get_info() const { return _info; }

			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			void accept(resource::IResourceVisitor& resourceVisitor) override;
			uint64_t get_size() override { return 0; }
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Sound"; }
			std::string get_type() override { return "sound"; }
		
		private:
			SoundInfo _info;
			UUID _uuid;
	};
}