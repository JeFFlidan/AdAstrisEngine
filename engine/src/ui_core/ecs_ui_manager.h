#pragma once

#include "ecs/type_info_table.h"
#include "ecs/entity_manager.h"
#include "core/math_base.h"
#include "core/reflection.h"
#include "engine_core/uuid.h"
#include <imgui/imgui.h>
#include <functional>

namespace ad_astris::uicore
{
	struct ComponentWidgetProperties
	{
		float minValueFloat{ 0.0f };
		float maxValueFloat{ 100000.0f };
		int32_t minValueInt{ 0 };
		int32_t maxValueInt{ 0 };
		float speed{ 0.1f };
	};
	
	XMFLOAT2 draw_element(ImGuiContext* context, const std::string& elementName, XMFLOAT2 vec, ComponentWidgetProperties& widgetProperties);
	XMFLOAT3 draw_element(ImGuiContext* context, const std::string& elementName, XMFLOAT3 vec, ComponentWidgetProperties& widgetProperties);
	XMFLOAT4 draw_element(ImGuiContext* context, const std::string& elementName, XMFLOAT4 vec, ComponentWidgetProperties& widgetProperties);
	UUID draw_element(ImGuiContext* context, const std::string& elementName, UUID uuid, ComponentWidgetProperties& widgetProperties);
	float draw_element(ImGuiContext* context, const std::string& elementName, float val, ComponentWidgetProperties& widgetProperties);
	double draw_element(ImGuiContext* context, const std::string& elementName, double val, ComponentWidgetProperties& widgetProperties);
	uint32_t draw_element(ImGuiContext* context, const std::string& elementName, uint32_t val, ComponentWidgetProperties& widgetProperties);
	uint64_t draw_element(ImGuiContext* context, const std::string& elementName, uint64_t val, ComponentWidgetProperties& widgetProperties);
	int32_t draw_element(ImGuiContext* context, const std::string& elementName, int32_t val, ComponentWidgetProperties& widgetProperties);
	int64_t draw_element(ImGuiContext* context, const std::string& elementName, int64_t val, ComponentWidgetProperties& widgetProperties);
	bool draw_element(ImGuiContext* context, const std::string& elementName, bool val, ComponentWidgetProperties& widgetProperties);
	
	class ECSUiManager
	{
		public:
			// Need point to the TypeInfoTable to set up it in the editor module.
			ECSUiManager(ecs::EntityManager* entityManager, ecs::TypeInfoTable* globalTypeInfoTable);
			void set_imgui_context(ImGuiContext* context);
		
			template<typename T>
			void register_component()
			{
				std::function<void(void*, ComponentWidgetProperties&)> func = [&](void* ptr, ComponentWidgetProperties& widgetProperties)
				{
					T& component = *static_cast<T*>(ptr);
					refl::util::for_each(refl::reflect(component).members, [&](auto member)
					{
						if constexpr (refl::descriptor::is_readable(member) && refl::descriptor::has_attribute<UIField>(member))
						{
							member(component, draw_element(_imGuiContext, refl::descriptor::get_display_name(member), member(component), widgetProperties));
						}
					});
				};
				_widgetDescByComponentID[ecs::TYPE_INFO_TABLE->get_component_id<T>()].callback = func;
			}

			template<typename T>
			void set_custom_callback(const std::function<void(void*, ComponentWidgetProperties&)>& customUICallback)
			{
				_widgetDescByComponentID[ecs::TYPE_INFO_TABLE->get_component_id<T>()].callback = customUICallback;
			}

			void draw_component(uint32_t componentID, void* component)
			{
				auto it = _widgetDescByComponentID.find(componentID);
				it->second.callback(component, it->second.widgetProperties);
			}

			template<typename T>
			void set_component_widget_properties(ComponentWidgetProperties& widgetProperties)
			{
				_widgetDescByComponentID[ecs::TYPE_INFO_TABLE->get_component_id<T>()].widgetProperties = widgetProperties;
			}

			// Must be used in all modules except the engine where the ECSUiManager is initialized
			void set_global_variables();

			template<typename T>
			bool does_entity_have_tag(ecs::Entity entity)
			{
				return _entityManager->does_entity_have_tag<T>(entity);
			}

			template<typename T>
			bool does_entity_have_component(ecs::Entity entity)
			{
				return _entityManager->does_entity_have_component<T>(entity);
			}

			void draw_component_widgets(ecs::Entity entity);

			template<typename T>
			const T* get_component_const(ecs::Entity entity)
			{
				return _entityManager->get_component_const<T>(entity);
			}

			template<typename T>
			T* get_component(ecs::Entity entity)
			{
				return _entityManager->get_component<T>(entity);
			}

		private:
			struct WidgetDesc
			{
				ComponentWidgetProperties widgetProperties;
				std::function<void(void*, ComponentWidgetProperties&)> callback;
			};
		
			std::unordered_map<uint32_t, WidgetDesc> _widgetDescByComponentID;
			ecs::TypeInfoTable* _typeInfoTable;
			ecs::EntityManager* _entityManager;
			ImGuiContext* _imGuiContext;
	};
}
