#pragma once

#include "events/event_manager.h"
#include "file_system/file_system.h"

namespace ad_astris::editor::impl
{
	struct UIWindowInitContext
	{
		events::EventManager* eventManager;
		io::FileSystem* fileSystem;
	};

	class UIWindowInternal
	{
		public:
			UIWindowInternal(UIWindowInitContext& initContext)
				: _eventManager(initContext.eventManager), _fileSystem(initContext.fileSystem) { }

		protected:
			events::EventManager* _eventManager{ nullptr };
			io::FileSystem* _fileSystem{ nullptr };
	};
}
