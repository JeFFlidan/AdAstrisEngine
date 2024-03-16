#pragma once

#include "engine_core/object.h"

namespace ad_astris::ecore
{
	struct ScriptInfo
	{
		
	};

	struct ScriptCreateInfo
	{
		
	};

	class Script : public Object
	{
		public:
			Script() = default;
			Script(const ScriptInfo& info, ObjectName* name);

			void set_info(const ScriptInfo& info) { _info = info; }
			const ScriptInfo& get_info() const { return _info; }

			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			void accept(resource::IResourceVisitor& resourceVisitor) override;
			uint64_t get_size() override { return 0; }
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Script"; }
			std::string get_type() override { return "script"; }
		
		private:
			ScriptInfo _info;
			UUID _uuid;
	};
}