#include "resource_loader.h"
#include "module_objects.h"
#include "core/global_objects.h"
#include "application_core/editor_events.h"

using namespace ad_astris;
using namespace engine::impl;

void ResourceLoader::init()
{
	subscribe_to_events();
}

void ResourceLoader::load_new_resources()
{
	tasks::TaskComposer* taskComposer = TASK_COMPOSER();
	resource::ResourceManager* resourceManager = RESOURCE_MANAGER();
	
	tasks::TaskGroup& taskGroup = *taskComposer->allocate_task_group();
	for (auto& paths : _resourcePaths)
	{
		taskComposer->execute(taskGroup, [this, paths, resourceManager](tasks::TaskExecutionInfo execInfo)
		{
			io::URI resourcePath = paths.first;
			io::URI aaresPath = paths.second;
			std::string fileExtension = io::Utils::get_file_extension(resourcePath);
			if (fileExtension == "jpg" || fileExtension == "tga" || fileExtension == "png")
				resourceManager->convert_to_aares<ecore::Texture2D>(resourcePath, aaresPath).get_resource();
			if (fileExtension == "obj")
				resourceManager->convert_to_aares<ecore::StaticModel>(resourcePath, aaresPath).get_resource();
		});
	}
	taskComposer->wait(taskGroup);
	taskComposer->free_task_group(&taskGroup);
	_resourcePaths.clear();

	for (auto& newMaterial : _materialsToCreate)
	{
		resourceManager->create_new_resource(newMaterial);
	}
	if (!_materialsToCreate.empty())
	{
		resourceManager->save_resources();
	}
	_materialsToCreate.clear();
}

void ResourceLoader::subscribe_to_events()
{
	events::EventManager* eventManager = EVENT_MANAGER();
	
	events::EventDelegate<acore::ResourceImportEvent> delegate1 = [&](acore::ResourceImportEvent& event)
	{
		_resourcePaths.push_back({ event.get_resource_path(), event.get_aares_path() });
	};
	eventManager->subscribe(delegate1);

	events::EventDelegate<acore::OpaquePBRMaterialCreationEvent> delegate2 = [&](acore::OpaquePBRMaterialCreationEvent& event)
	{
		resource::FirstCreationContext<ecore::OpaquePBRMaterial> firstCreationContext;
		firstCreationContext.materialName = event.get_material_name();
		firstCreationContext.materialPath = event.get_material_path();
		firstCreationContext.materialSettings.baseColorTextureUUID = event.get_albedo_texture_uuid();
		firstCreationContext.materialSettings.normalTextureUUID = event.get_normal_texture_uuid();
		firstCreationContext.materialSettings.roughnessTextureUUID = event.get_roughness_texture_uuid();
		firstCreationContext.materialSettings.metallicTextureUUID = event.get_metallic_texture_uuid();
		firstCreationContext.materialSettings.ambientOcclusionTextureUUID = event.get_ao_texture_uuid();
		_materialsToCreate.push_back(firstCreationContext);
	};
	eventManager->subscribe(delegate2);
}
