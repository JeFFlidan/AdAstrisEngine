#include "engine.h"
#include "engine/public/engine_module.h"

namespace ad_astris::engine::impl
{
	class EngineModule : public IEngineModule
	{
		public:
			virtual void startup_module(ModuleManager* moduleManager) override;
			virtual IEngine* get_engine() override;

		private:
			std::unique_ptr<Engine> _engine{ nullptr };
	};

	void EngineModule::startup_module(ModuleManager* moduleManager)
	{
		_engine = std::make_unique<Engine>();
	}

	IEngine* EngineModule::get_engine()
	{
		return _engine.get();
	}

	DECL_MODULE_FUNCS(Engine, EngineModule)
}