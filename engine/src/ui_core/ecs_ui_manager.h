#pragma once

#include "ecs/type_info_table.h"
#include "ecs/entity_manager.h"
#include "core/attributes.h"
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
			void set_imgui_context(ImGuiContext* context);
		
			template<typename T>
			void register_component()
			{
				std::function<void(void*, ComponentWidgetProperties&)> func = [&](void* ptr, ComponentWidgetProperties& widgetProperties)
				{
					T& component = *static_cast<T*>(ptr);
					Reflector::for_each<T>([&](auto field)
					{
						if constexpr (Reflector::has_attribute<EditAnywhere>(field))
						{
							field(component, draw_element(_imGuiContext, Reflector::get_name(field), field(component), widgetProperties));
						}
					});
				};
				_widgetDescByComponentID[ecs::TypeInfoTable::get_component_id<T>()].callback = func;
			}

			template<typename T>
			void set_custom_callback(const std::function<void(void*, ComponentWidgetProperties&)>& customUICallback)
			{
				_widgetDescByComponentID[ecs::TypeInfoTable::get_component_id<T>()].callback = customUICallback;
			}

			void draw_component(uint64_t componentID, void* component)
			{
				auto it = _widgetDescByComponentID.find(componentID);
				it->second.callback(component, it->second.widgetProperties);
			}

			template<typename T>
			void set_component_widget_properties(ComponentWidgetProperties& widgetProperties)
			{
				_widgetDescByComponentID[ecs::TypeInfoTable::get_component_id<T>()].widgetProperties = widgetProperties;
			}

			// Must be used in all modules except the engine where the ECSUiManager is initialized
			void set_global_variables();

			void draw_component_widgets(ecs::Entity entity);

		private:
			struct WidgetDesc
			{
				ComponentWidgetProperties widgetProperties;
				std::function<void(void*, ComponentWidgetProperties&)> callback;
			};
		
			std::unordered_map<uint64_t, WidgetDesc> _widgetDescByComponentID;
			ImGuiContext* _imGuiContext;
	};
}
