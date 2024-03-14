#pragma once

#include "api.h"
#include "resource_table.h"
#include "resource_manager/resource_manager2.h"

namespace ad_astris::resource::impl
{
	class RESOURCE_MANAGER_API ResourceManager : public experimental::ResourceManager
	{
		public:
			void init(const experimental::ResourceManagerInitContext& initContext) override;
			void cleanup() override;

			UUID convert_to_engine_format(const io::URI& originalResourcePath, const io::URI& engineResourcePath) override;
			void save_resources() const override;
			void destroy_resource(UUID uuid) override;
		
			ResourceAccessor<ecore::Model> get_model(UUID uuid) const override;
			ResourceAccessor<ecore::Model> get_model(const std::string& modelName) const override;
			ResourceAccessor<ecore::Texture> get_texture(UUID uuid) const override;
			ResourceAccessor<ecore::Texture> get_texture(const std::string& textureName) const override;
			ResourceAccessor<ecore::Level> get_level(UUID uuid) const override;
			ResourceAccessor<ecore::Level> get_level(const std::string& levelName) const override;
		
			ResourceType get_resource_type(UUID uuid) const override;
			std::string get_resource_name(UUID uuid) const override;
			UUID get_resource_uuid(const std::string& resourceName) const override;
			bool is_resource_loaded(UUID uuid) const override;
			bool is_supported_model_format(const std::string& extension) const override;
			bool is_supported_texture_format(const std::string& extension) const override;

		private:
			std::unique_ptr<ResourcePool> _resourcePool;
			std::unique_ptr<ResourceTable> _resourceTable;
	};
}