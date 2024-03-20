#pragma once

#include "core/global_objects.h"
#include "core/module.h"
#include "application_core/window.h"
#include "rhi/ui_window_backend.h"
#include "ui_core/ecs_ui_manager.h"

namespace ad_astris::editor
{
	struct EditorInitContext
	{
		acore::IWindow* mainWindow{ nullptr };
		uicore::ECSUiManager* ecsUiManager{ nullptr };
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