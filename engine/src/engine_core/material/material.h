#pragma once

#include "engine_core/object.h"

namespace ad_astris::ecore
{
	class Material : public Object
	{
		public:
			Material() = default;

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

		protected:
			virtual void rename_in_engine(ObjectName& newName) override;

			// ========== End Object interface ==========
	};
}