#pragma once

#include "engine_core/object.h"
#include <vector>

namespace ad_astris::ecore
{
	class GameObject;
	
	class Level : Object
	{
		// TODO
		public:
			Level() = default;

		private:
			//std::vector<GameObject> _gameObjects;
		
		public:
			// ========== Begin Object interface ==========
			
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file) override;
			virtual uint64_t get_size() override;
			virtual bool is_resource() override;
			virtual UUID get_uuid() override;
			virtual std::string get_description() override;
			virtual std::string get_type() override;
		
		protected:
			virtual void rename_in_engine(ObjectName& newName) override;

			// ========== End Object interface ==========
	};
}
