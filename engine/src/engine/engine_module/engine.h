#pragma once

#include "api.h"
#include "project.h"
#include "engine_objects_creator.h"
#include "resource_loader.h"
#include "engine/engine_module.h"
#include "application_core/window.h"
#include "events/event_manager.h"
#include "engine/renderer_module.h"
#include "file_system/file_system.h"
#include "engine_core/basic_events.h"

namespace ad_astris::engine::impl
{
	class ENGINE_API Engine final : public IEngine
	{
		public:
			virtual void init(EngineInitializationContext& engineInitContext) override;
			virtual void execute() override;
			virtual void save_and_cleanup(bool needToSave) override;

		private:
			acore::IWindow* _mainWindow{ nullptr };
			renderer::IRenderer* _renderer{ nullptr };
			std::unique_ptr<Project> _project{ nullptr };
			std::unique_ptr<EngineObjectsCreator> _engineObjectsCreator{ nullptr };
			std::unique_ptr<ResourceLoader> _resourceLoader{ nullptr };

			ecs::Entity _activeCamera;
			events::EventDelegate<ecore::EntityCreatedEvent> _activeCameraDelegate;

			void init_global_objects(GlobalObjectContext* context);
			void init_module_objects();
			void init_local_objects();
			void init_renderer(EngineInitializationContext& engineInitContext);
			void register_ecs_objects();
			void pre_update();
			void set_active_camera_delegate();
			void subscribe_to_events();
	};
}