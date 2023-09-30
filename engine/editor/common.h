#pragma once

#include "events/event_manager.h"
#include "resource_manager/resource_formats.h"
#include "file_system/file_system.h"
#include "ui_core/ecs_ui_manager.h"

namespace ad_astris::editor::impl
{
	struct ResourceInfo
	{
		UUID uuid;
		resource::ResourceType type;
	};

	struct ResourceDesc
	{
		UUID uuid;
		std::string resourceName;
	};
	struct UIWindowInitContext
	{
		events::EventManager* eventManager;
		io::FileSystem* fileSystem;
		uicore::ECSUiManager* ecsUiManager;
		std::unordered_map<std::string, ResourceInfo>* resourceInfoByRelativePath;
		std::unordered_map<resource::ResourceType, std::vector<ResourceDesc>>* resourceNamesBytType;
	};

	class UIWindowInternal
	{
		public:
			UIWindowInternal(UIWindowInitContext& initContext)
				: _eventManager(initContext.eventManager), _fileSystem(initContext.fileSystem), _ecsUiManager(initContext.ecsUiManager),
				_resourceInfoByRelativePath(*initContext.resourceInfoByRelativePath), _resourceDescriptionsByType(*initContext.resourceNamesBytType){ }

		protected:
			events::EventManager* _eventManager{ nullptr };
			io::FileSystem* _fileSystem{ nullptr };
			uicore::ECSUiManager* _ecsUiManager{ nullptr };
			std::unordered_map<std::string, ResourceInfo>& _resourceInfoByRelativePath;
			std::unordered_map<resource::ResourceType, std::vector<ResourceDesc>>& _resourceDescriptionsByType;
	};

	class MaterialCreationWindowDrawEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(MaterialCreationWindowDrawEvent)
			MaterialCreationWindowDrawEvent(io::URI currentDirectory) : _currentDirectory(currentDirectory) { }

			io::URI get_current_directory() { return _currentDirectory; }

		private:
			io::URI _currentDirectory;
	};

	class ModelCreationWindowDrawEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ModelCreationWindowDrawEvent)
	};
}
