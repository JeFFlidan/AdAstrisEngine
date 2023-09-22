#pragma once

#include "api.h"
#include "ui_core/common.h"
#include "application/editor_module.h"
#include <vulkan/vulkan.h>

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
			std::vector<std::shared_ptr<uicore::IUIWindow>> _uiWindows;
			std::function<void()> _uiBeginFrameCallback{ nullptr };
			std::function<void()> _setContextCallback{ nullptr };
			
	};
}
