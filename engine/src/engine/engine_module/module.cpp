#include "engine.h"
#include "engine/engine_module.h"

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

	extern "C" ENGINE_API IEngineModule* register_module()
	{
		return new EngineModule();
	}
}