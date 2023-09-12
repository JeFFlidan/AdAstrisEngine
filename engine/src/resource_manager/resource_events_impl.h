#pragma once

#include "resource_formats.h"
#include "engine_core/material/material_template.h"
#include "engine_core/material/materials.h"
#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"

namespace ad_astris::resource::impl
{
	class StaticModelEvent
	{
		public:
			StaticModelEvent(ecore::StaticModel* staticModel) : _model(staticModel) { }

			ResourceAccessor<ecore::StaticModel> get_model_handle()
			{
				return _model;
			}

		private:
			ecore::StaticModel* _model;
	};

	class Texture2DEvent
	{
		public:
			Texture2DEvent(ecore::Texture2D* texture2D) : _texture(texture2D) { }

			ResourceAccessor<ecore::Texture2D> get_texture_handle()
			{
				return _texture;
			}

		private:
			ecore::Texture2D* _texture;
	};
	
	class MaterialTemplateEvent
	{
		public:
			MaterialTemplateEvent(ecore::MaterialTemplate* materialTemplate) : _materialTemplate(materialTemplate) { }

			ResourceAccessor<ecore::MaterialTemplate> get_material_template_handle()
			{
				return _materialTemplate;
			}

		private:
			ecore::MaterialTemplate* _materialTemplate;
	};
}
