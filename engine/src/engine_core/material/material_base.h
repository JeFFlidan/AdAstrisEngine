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
			MaterialBase(ObjectName* materialName, UUID generalMaterialTemplateUUID);

		protected:
			std::unique_ptr<IMaterialSettings> _materialSettings;
			UUID _materialUUID;
			UUID _materialTemplateUUID;

		public:
			// ========== Begin Object interface ==========

			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* objectName) override;
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
