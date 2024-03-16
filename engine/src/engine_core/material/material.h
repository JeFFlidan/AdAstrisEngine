#pragma once

#include "engine_core/object.h"

namespace ad_astris::ecore
{
	struct MaterialInfo
	{
		
	};

	struct MaterialCreateInfo
	{
		
	};

	class Material : public Object
	{
		public:
			Material() = default;
			Material(const MaterialInfo& info, ObjectName* name);

			void set_info(const MaterialInfo& info) { _info = info; }
			const MaterialInfo& get_info() const { return _info; }

			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			void accept(resource::IResourceVisitor& resourceVisitor) override;
			uint64_t get_size() override { return 0; }
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Material"; }
			std::string get_type() override { return "material"; }
		
		private:
			MaterialInfo _info;
			UUID _uuid;
	};
}