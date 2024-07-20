#pragma once

#include "common.h"
#include "ui_core/common.h"
#include <functional>

namespace ad_astris::editor::impl
{
	class ViewportWindow : public uicore::IUIWindow, public UIWindowInternal
	{
		public:
			ViewportWindow(UIWindowInitContext& initContext);
			virtual void draw_window(void* data = nullptr) override;

			void set_selected_entity(ecs::Entity entity) { _selectedEntity = entity; }	// Temporary solution until implementing mouse picking

		private:
			uint64_t _textureIndex;
			ImVec2 _viewportExtent{ 0, 0 };
			ecs::Entity _selectedEntity;
			ecs::Entity _activeCamera;
			int32_t _gizmoType{ -1 };

			void draw_gizmo();
			void subscribe_to_events();
	};
}
