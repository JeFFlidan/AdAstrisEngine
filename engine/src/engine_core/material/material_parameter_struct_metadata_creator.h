#pragma once

#include "material_parameter_struct_metadata.h"

namespace ad_astris::ecore
{
	class MaterialParameterStructMetadataCreator
	{
		public:
			static std::unique_ptr<MaterialParameterStructMetadata> create(const io::URI& path);
			static void update(MaterialParameterStructMetadata* metadata);
	};
}