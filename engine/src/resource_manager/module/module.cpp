#include "resource_manager.h"
#include "resource_manager/resource_manager_module.h"

namespace ad_astris::resource::impl
{
	class ResourceManagerModule : public experimental::IResourceManagerModule
	{
		public:
			void startup_module(ModuleManager* moduleManager) override
			{
				_resourceManager = std::make_unique<ResourceManager>();
			}
		
			experimental::ResourceManager* get_resource_manager() override
			{
				return _resourceManager.get();
			}

		private:
			std::unique_ptr<ResourceManager> _resourceManager{ nullptr };
	};

	extern "C" RESOURCE_MANAGER_API experimental::IResourceManagerModule* register_module()
	{
		return new ResourceManagerModule();
	}
}