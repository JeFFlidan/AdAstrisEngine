#pragma once

#include "engine_core/object.h"
#include "material_settings.h"
#include <memory>

namespace ad_astris::ecore
{
	class MaterialBase : public Object
	{
		public:
			MaterialBase() = default;
			MaterialBase(ObjectName* materialName);

		protected:
			std::unique_ptr<IMaterialSettings> _materialSettings;
			UUID _materialUUID;

		public:
			// ========== Begin Object interface ==========

			virtual void serialize(io::File* file) override;
			virtual void deserialize(io::File* file, ObjectName* objectName) override;
			virtual bool is_resource() override
			{
				return true;
			}
		
			virtual UUID get_uuid() override
			{
				return _materialUUID;
			}
		
			virtual std::string get_description() override
			{
				// TODO
				return std::string();
			}
		
			virtual std::string get_type() override
			{
				return "material";
			}

			// ========== End Object interface ==========
	};
}
