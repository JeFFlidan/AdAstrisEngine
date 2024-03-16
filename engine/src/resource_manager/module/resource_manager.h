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

			std::vector<UUID> convert_to_engine_format(
				const io::URI& originalResourcePath,
				const io::URI& engineResourcePath,
				void* conversionContext) override;
			void save_resources() const override;
			void save_resource(UUID uuid) const override;
			void save_resource(const std::string& name) const override;
			void unload_resource(UUID uuid) override;
			void unload_resource(const std::string& name) override;
			void destroy_resource(UUID uuid) override;
			void destroy_resource(const std::string& name) override;

			ResourceAccessor<ecore::Level> create_level(const ecore::LevelCreateInfo& createInfo) override;
			ResourceAccessor<ecore::Material> create_material(const ecore::MaterialCreateInfo& createInfo) override;
			ResourceAccessor<ecore::Script> create_script(const ecore::ScriptCreateInfo& createInfo) override;
		
			ResourceAccessor<ecore::Model> get_model(UUID uuid) const override;
			ResourceAccessor<ecore::Model> get_model(const std::string& modelName) const override;
			ResourceAccessor<ecore::Texture> get_texture(UUID uuid) const override;
			ResourceAccessor<ecore::Texture> get_texture(const std::string& textureName) const override;
			ResourceAccessor<ecore::Level> get_level(UUID uuid) const override;
			ResourceAccessor<ecore::Level> get_level(const std::string& levelName) const override;
			ResourceAccessor<ecore::Material> get_material(UUID uuid) const override;
			ResourceAccessor<ecore::Material> get_material(const std::string& materialName) const override;
			ResourceAccessor<ecore::Script> get_script(UUID uuid) const override;
			ResourceAccessor<ecore::Script> get_script(const std::string& scriptName) const override;
			ResourceAccessor<ecore::Video> get_video(UUID uuid) const override;
			ResourceAccessor<ecore::Video> get_video(const std::string& videoName) const override;
			ResourceAccessor<ecore::Font> get_font(UUID uuid) const override;
			ResourceAccessor<ecore::Font> get_font(const std::string& fontName) const override;
			ResourceAccessor<ecore::Sound> get_sound(UUID uuid) const override;
			ResourceAccessor<ecore::Sound> get_sound(const std::string& soundName) const override;
		
			ResourceType get_resource_type(UUID uuid) const override;
			std::string get_resource_name(UUID uuid) const override;
			UUID get_resource_uuid(const std::string& resourceName) const override;
			bool is_resource_loaded(UUID uuid) const override;
			bool is_model_format_supported(const std::string& extension) const override;
			bool is_texture_format_supported(const std::string& extension) const override;
			bool is_script_format_supported(const std::string& extension) const override;
			bool is_video_format_supported(const std::string& extension) const override;
			bool is_font_format_supported(const std::string& extension) const override;
			bool is_sound_format_supported(const std::string& extension) const override;

		private:
			std::unique_ptr<ResourcePool> _resourcePool;
			std::unique_ptr<ResourceTable> _resourceTable;
	};
}