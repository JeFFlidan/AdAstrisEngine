#pragma once

#include "common.h"
#include "ui_core/common.h"
#include "ecs/entity_types.h"

namespace ad_astris::editor::impl
{
	class OutlinerWindow : public uicore::IUIWindow, public UIWindowInternal
	{
		public:
			OutlinerWindow(UIWindowInitContext& initContext);
			virtual void draw_window(void* data = nullptr) override;
			ecs::Entity get_last_selected_entity() { return _entityByItsName[_lastSelectedEntityName]; }

		private:
			std::string _lastSelectedEntityName;
			std::vector<std::string> _entityNames;
			std::unordered_map<std::string, ecs::Entity> _entityByItsName;
			std::unordered_map<std::string, uint32_t> _nameCounterByDefaultName;
			bool _areAnyEntitiesSelected{ false };

			void subscribe_to_events();
			void generate_entity_name(ecs::Entity entity);
			void draw_entity_node(std::string& entityName);
	};
}