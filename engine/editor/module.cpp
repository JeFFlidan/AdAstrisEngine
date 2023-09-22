#include "editor.h"
#include "application/editor_module.h"

namespace ad_astris::editor
{
	class EditorModule : public IEditorModule
	{
		public:
			virtual void startup_module(ModuleManager* moduleManager) override;
			virtual IEditor* get_editor() override;

		private:
			std::unique_ptr<impl::Editor> _editor{ nullptr };
	};

	void EditorModule::startup_module(ModuleManager* moduleManager)
	{
		_editor = std::make_unique<impl::Editor>();
	}

	IEditor* EditorModule::get_editor()
	{
		return _editor.get();
	}

	extern "C" EDITOR_API IEditorModule* register_module()
	{
		return new EditorModule();
	}
}