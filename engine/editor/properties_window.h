#pragma once

#include "common.h"
#include "ui_core/common.h"

namespace ad_astris::editor::impl
{
	class PropertiesWindow : public uicore::IUIWindow, public UIWindowInternal
	{
		public:
			PropertiesWindow(UIWindowInitContext& initContext);
			virtual void draw_window(void* data = nullptr) override;
			void set_selected_entity(ecs::Entity entity) { _selectedEntity = entity; }

		private:
			ecs::Entity _selectedEntity;

			void set_transform_component_widgets();
			void set_color_component_widget();
			void set_material_component_widget();
	};
}