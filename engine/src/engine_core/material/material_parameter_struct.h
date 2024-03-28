#pragma once

#include "material_parameter.h"

namespace ad_astris::ecore
{
	class MaterialParameterStructMetadata;
	
	class MaterialParameterStruct
	{
		public:
			MaterialParameterStruct(MaterialParameterStructMetadata* metadata);

			void add_parameter(MaterialParameterMetadata* parameterMetadata);
			void remove_parameter(MaterialParameterMetadata* parameterMetadata);
		
			MaterialParameterStructMetadata* get_metadata() const;

		private:
			MaterialParameterStructMetadata* _metadata{ nullptr };
			std::vector<std::unique_ptr<MaterialParameter>> _parameters;
	};
}