#pragma once

#include "common.h"
#include "resource_manager/resource_manager.h"
#include "resource_manager/resource_manager2.h"
#include "multithreading/task_composer.h"
#include "events/event_manager.h"
#include "file_system/file_system.h"
#include "engine_core/world.h"
#include "ecs/system_manager.h"
#include "module_manager.h"
#include "profiler/profiler.h"
#include "ui_core/ecs_ui_manager.h"
#include "rhi/imgui_backend.h"

namespace ad_astris
{
	struct GlobalObjectContext
	{
		std::unique_ptr<io::FileSystem> fileSystem{ nullptr };
		std::unique_ptr<ModuleManager> moduleManager{ nullptr };
		std::unique_ptr<tasks::TaskComposer> taskComposer{ nullptr };
		std::unique_ptr<resource::ResourceManager> resourceManager{ nullptr };
		std::unique_ptr<events::EventManager> eventManager{ nullptr };
		std::unique_ptr<ecore::World> world{ nullptr };
		std::unique_ptr<ecs::SystemManager> systemManager{ nullptr };
		std::unique_ptr<ecs::TypeInfoTable> ecsTypeInfoTable{ nullptr };
		std::unique_ptr<profiler::ProfilerInstance> profilerInstance{ nullptr };
		std::unique_ptr<uicore::ECSUiManager> ecsUIManager{ nullptr };
		rhi::IImGuiBackend* imguiBackend{ nullptr };
	};
	
	class  GlobalObjects
	{
		public:
			static void init_file_system();
			static void init_module_manager();
			static void init_task_composer(uint32_t threadCount = ~0u);
			static void init_event_manager();
			static void init_resource_manager();
			static void init_world();
			static void init_system_manager();
			static void init_profiler_instance();
		
			static void set_global_object_context(GlobalObjectContext* context)
			{
				assert(context);
				_globalObjectContext = context;
			}

			static GlobalObjectContext* get_global_object_context()
			{
				return _globalObjectContext;
			}

			static void set_imgui_backend(rhi::IImGuiBackend* imguiBackend)
			{
				_globalObjectContext->imguiBackend = imguiBackend;
			}
		
			FORCE_INLINE static io::FileSystem* get_file_system() { return _globalObjectContext->fileSystem.get(); }
			FORCE_INLINE static tasks::TaskComposer* get_task_composer() { return _globalObjectContext->taskComposer.get(); }
			FORCE_INLINE static resource::ResourceManager* get_resource_manager() { return _globalObjectContext->resourceManager.get(); }
			FORCE_INLINE static events::EventManager* get_event_manager() { return _globalObjectContext->eventManager.get(); }
			FORCE_INLINE static ecore::World* get_world() { return _globalObjectContext->world.get(); }
			FORCE_INLINE static ecs::SystemManager* get_system_manager() { return _globalObjectContext->systemManager.get(); }
			FORCE_INLINE static ecs::TypeInfoTable* get_ecs_type_info_table() { return _globalObjectContext->ecsTypeInfoTable.get(); }
			FORCE_INLINE static ModuleManager* get_module_manager() { return _globalObjectContext->moduleManager.get(); }
			FORCE_INLINE static profiler::ProfilerInstance* get_profiler_instance() { return _globalObjectContext->profilerInstance.get(); }
			FORCE_INLINE static uicore::ECSUiManager* get_ecs_ui_manager() { return _globalObjectContext->ecsUIManager.get(); }
			FORCE_INLINE static rhi::IImGuiBackend* get_imgui_backend() { return _globalObjectContext->imguiBackend; }
		
		private:
			inline static GlobalObjectContext* _globalObjectContext{ nullptr };
	};
}

#define FILE_SYSTEM() ::ad_astris::GlobalObjects::get_file_system()
#define TASK_COMPOSER() ::ad_astris::GlobalObjects::get_task_composer()
#define RESOURCE_MANAGER() ::ad_astris::GlobalObjects::get_resource_manager()
#define EVENT_MANAGER() ::ad_astris::GlobalObjects::get_event_manager()
#define WORLD() ::ad_astris::GlobalObjects::get_world()
#define SYSTEM_MANAGER() ::ad_astris::GlobalObjects::get_system_manager()
#define ECS_TYPE_INFO_TABLE() ::ad_astris::GlobalObjects::get_ecs_type_info_table()
#define MODULE_MANAGER() ::ad_astris::GlobalObjects::get_module_manager()
#define PROFILER_INSTANCE() ::ad_astris::GlobalObjects::get_profiler_instance()
#define ECS_UI_MANAGER() ::ad_astris::GlobalObjects::get_ecs_ui_manager()
#define ENTITY_MANAGER() WORLD()->get_entity_manager()
#define IMGUI_BACKEND() ::ad_astris::GlobalObjects::get_imgui_backend()
