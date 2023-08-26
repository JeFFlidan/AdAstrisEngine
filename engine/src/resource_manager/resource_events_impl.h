#pragma once

#include "resource_formats.h"
#include "engine_core/material/material_template.h"
#include "engine_core/material/materials.h"

namespace ad_astris::resource::impl
{
	class MaterialTemplateEvent
	{
		public:
			MaterialTemplateEvent(ecore::MaterialTemplate* materialTemplate) : _materialTemplate(materialTemplate) { }

			ResourceAccessor<ecore::MaterialTemplate> get_material_template()
			{
				return _materialTemplate;
			}

		private:
			ecore::MaterialTemplate* _materialTemplate;
	};
}
