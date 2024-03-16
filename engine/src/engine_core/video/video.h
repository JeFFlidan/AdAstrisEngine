#pragma once

#include "engine_core/object.h"

namespace ad_astris::ecore
{
	struct VideoInfo
	{
		
	};

	class Video : public Object
	{
		public:
			Video() = default;
			Video(const VideoInfo& info, ObjectName* name);

			void set_info(const VideoInfo& info) { _info = info; }
			const VideoInfo& get_info() const { return _info; }

			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			void accept(resource::IResourceVisitor& resourceVisitor) override;
			uint64_t get_size() override { return 0; }
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Video"; }
			std::string get_type() override { return "video"; }
		
		private:
			VideoInfo _info;
			UUID _uuid;
	};
}