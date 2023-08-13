#pragma once

#include "api.h"
#include "engine/engine_module.h"
#include "application_core/window.h"
#include "engine_core/project_settings/project_settings.h"
#include "engine_core/world.h"
#include "events/event_manager.h"
#include "engine/renderer_module.h"
#include "resource_manager/resource_manager.h"
#include "file_system/file_system.h"
#include <memory>

namespace ad_astris::engine::impl
{
	class ENGINE_API Engine final : public IEngine
	{
		public:
			virtual void init(EngineInitializationContext& initializationContext) override;
			virtual void save_and_cleanup(bool needToSave) override;

		private:
			io::FileSystem* _fileSystem{ nullptr };
			ModuleManager* _moduleManager{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			acore::IWindow* _mainWindow{ nullptr };
			std::unique_ptr<resource::ResourceManager> _resourceManager{ nullptr };
			renderer::IRenderer* _renderer{ nullptr };
			std::unique_ptr<ecore::World> _world{ nullptr };
			std::unique_ptr<ecore::ProjectSettings> _projectSettings{ nullptr };
		
			void create_new_blank_project();
			void load_existing_project();
	};
}