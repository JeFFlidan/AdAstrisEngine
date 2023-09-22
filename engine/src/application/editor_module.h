#pragma once

#include "core/module.h"
#include "events/event_manager.h"
#include "application_core/window.h"
#include "file_system/file_system.h"
#include "rhi/ui_window_backend.h"
#include <functional>

namespace ad_astris::editor
{
	struct EditorInitContext
	{
		acore::IWindow* mainWindow{ nullptr };
		io::FileSystem* fileSystem{ nullptr };
		events::EventManager* eventManager{ nullptr };
		rhi::UIWindowBackendCallbacks* callbacks{ nullptr };
	};
	
	class IEditor
	{
		public:
			virtual ~IEditor() {}
			virtual void init(EditorInitContext& initContext) = 0;
			virtual void cleanup() = 0;
			virtual void draw() = 0;
	};
	
	class IEditorModule : public IModule
	{
		public:
			virtual IEditor* get_editor() = 0;
	};
}