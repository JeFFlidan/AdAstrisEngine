#pragma once

#include "api.h"
#include "ui_core/common.h"
#include "application/editor_module.h"
#include "ui_core/docking_window.h"
#include "viewport_window.h"
#include "outliner_window.h"
#include "properties_window.h"
#include "content_browser_window.h"
#include "material_creation_window.h"
#include "model_creation_window.h"

namespace ad_astris::editor::impl
{
	class EDITOR_API Editor : public IEditor
	{
		public:
			virtual void init(EditorInitContext& initContext) override;
			virtual void cleanup() override;
			virtual void draw() override;

		private:
			io::FileSystem* _fileSystem{ nullptr };
			events::EventManager* _eventManager{ nullptr };
			uicore::ECSUiManager* _ecsUiManager{ nullptr };
			uicore::DockingWindow _dockingWindow;
			std::unique_ptr<ViewportWindow> _viewportWindow{ nullptr };
			std::unique_ptr<OutlinerWindow> _outlinerWindow{ nullptr };
			std::unique_ptr<PropertiesWindow> _propertiesWindow{ nullptr };
			std::unique_ptr<ContentBrowserWindow> _contentBrowserWindow{ nullptr };
			std::unique_ptr<MaterialCreationWindow> _materialCreationWindow{ nullptr };
			std::unique_ptr<ModelCreationWindow> _modelCreationWindow{ nullptr };
			std::function<void()> _uiBeginFrameCallback{ nullptr };
			std::function<void()> _setContextCallback{ nullptr };
			std::function<ImFont*()> _getDefaultFontSize14;
			std::function<ImFont*()> _getDefaultFontSize17;
		
			std::unordered_map<std::string, ResourceInfo> _resourceInfoByRelativePath;
			std::unordered_map<resource::ResourceType, std::vector<ResourceDesc>> _resourceDescriptionsByType;
			std::mutex _resourceInfoMutex;

			void subscribe_to_events();
	};
}
