#include "project.h"
#include "engine_core/default_object_creators.h"
#include "engine_core/basic_components.h"
#include "core/global_objects.h"
#include "engine_core/contexts.h"
#include "engine_core/material/materials.h"

using namespace ad_astris;
using namespace engine::impl;

void Project::load(const devtools::ProjectInfo& projectInfo)
{
	switch (projectInfo.newProjectTemplate)
	{
		case devtools::NewProjectTemplate::OLD_PROJECT:
		{
			load_existing_project();
			LOG_INFO("Engine::init(): Loaded an existing project {}", _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>()->get_project_name())
			break;
		}
		case devtools::NewProjectTemplate::BLANK:
		{
			create_new_blank_project();
			LOG_INFO("Engine::init(): Created a blank project {}", _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>()->get_project_name())
			break;
		}
	}
}

void Project::postload(const devtools::ProjectInfo& projectInfo)
{
	if (projectInfo.newProjectTemplate == devtools::NewProjectTemplate::OLD_PROJECT)
	{
		RESOURCE_MANAGER()->load_builtin_resources();
		LOG_INFO("Engine::init(): Loaded builtin resources")
	}
	else
	{
		//create_material_templates();
		LOG_INFO("Engine::init(): Created material templates for new project")
	}
}

void Project::save()
{
	_projectSettings->serialize(FILE_SYSTEM());
}

void Project::load_existing_project()
{
	_projectSettings = std::make_unique<ecore::ProjectSettings>();
	io::URI projectRootPath = FILE_SYSTEM()->get_project_root_path();
	_projectSettings->deserialize(projectRootPath + "/configs/project_settings.ini");
	auto projectDescriptionSubsettings = _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>();
	io::URI levelRelativePath = projectDescriptionSubsettings->get_default_level_path().c_str();
	io::URI levelPath = io::Utils::get_absolute_path_to_file(projectRootPath, levelRelativePath);
	ecore::LevelHandle level = RESOURCE_MANAGER()->load_level(levelPath);
	WORLD()->add_level(level.get_resource(), true, true);
	level.get_resource()->build_entities();
	LOG_INFO("Engine::init(): Loaded default level {}", level.get_resource()->get_name()->get_full_name())
}

void Project::create_new_blank_project()
{
	const io::URI projectRootPath = FILE_SYSTEM()->get_project_root_path();
	const io::URI engineRootPath = FILE_SYSTEM()->get_engine_root_path();
	
	io::URI path = projectRootPath + "/content/levels/base_level.aalevel";
	ecore::LevelHandle level = RESOURCE_MANAGER()->create_level(path);
	WORLD()->add_level(level.get_resource(), true, true);
	LOG_INFO("Engine::init(): Created new level")

	ecore::DefaultSettingsContext<ecore::ProjectSettings> defaultSettingsContext;
	io::URI relativePathToDefaultLevel = io::Utils::get_relative_path_to_file(projectRootPath, path);
	io::Utils::replace_back_slash_to_forward(relativePathToDefaultLevel);
	defaultSettingsContext.defaultLevelPath = relativePathToDefaultLevel;
	io::URI aaprojectFilePath = io::Utils::find_file_with_specific_extension(projectRootPath, ".aaproject");
	defaultSettingsContext.projectName = io::Utils::get_file_name(aaprojectFilePath);
	_projectSettings = std::make_unique<ecore::ProjectSettings>();
	_projectSettings->setup_default_settings(defaultSettingsContext);
	_projectSettings->deserialize(projectRootPath + "/configs/project_settings.ini");
	_projectSettings->serialize(FILE_SYSTEM());
	
	create_default_material(engineRootPath, projectRootPath);
	
	ecore::EditorObjectCreationContext editorObjectCreationContext;
	io::URI modelPath = engineRootPath + "/starter_content/models/plane.obj";
	resource::ResourceAccessor<ecore::StaticModel> model = RESOURCE_MANAGER()->convert_to_aares<ecore::StaticModel>(modelPath, "content");
	editorObjectCreationContext.uuid = model.get_resource()->get_uuid();
	editorObjectCreationContext.location = XMFLOAT3(0.0f, 0.0f, 0.0f);
	editorObjectCreationContext.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	editorObjectCreationContext.materialUUID = get_default_material_uuid();
	ecs::Entity entity = ecore::StaticModelCreator::create(editorObjectCreationContext);
	
	ecore::EditorObjectCreationContext creationContext{ };
	creationContext.location = XMFLOAT3(0.0f, 35.0f, 0.0f);
	ecs::Entity activeCamera = ecore::CameraCreator::create(creationContext);
	auto cameraComponent = WORLD()->get_entity_manager()->get_component<ecore::CameraComponent>(activeCamera);
	cameraComponent->isActive = true;
	
	ecore::StaticDirectionalLightCreator::create(creationContext);
	creationContext.location = XMFLOAT3(12.0f, 100.0f, 25.0f);
	ecore::StaticPointLightCreator::create(creationContext);

	RESOURCE_MANAGER()->save_resources();
	
	LOG_INFO("Engine::init(): Saved project with default settings")
}

void Project::create_default_material(const io::URI& engineRootPath, const io::URI& projectRootPath)
{
	tasks::TaskGroup taskGroup;
	ecore::Texture2D* albedo, *normal, *roughness, *metallic, *ao;
	
	std::vector<io::URI> paths = {
		engineRootPath + "/starter_content/textures/DustyCobbleAlbedo.tga",
		engineRootPath + "/starter_content/textures/DustyCobbleAO.tga",
		engineRootPath + "/starter_content/textures/DustyCobbleNormal.tga",
		engineRootPath + "/starter_content/textures/DustyCobbleRoughness.tga",
		engineRootPath + "/starter_content/textures/BlackMetallic.tga",
	};

	tasks::TaskComposer* taskComposer = TASK_COMPOSER();
	resource::ResourceManager* resourceManager = RESOURCE_MANAGER();
	
	taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		albedo = resourceManager->convert_to_aares<ecore::Texture2D>(paths[0], "content").get_resource();
	});
	taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		ao = resourceManager->convert_to_aares<ecore::Texture2D>(paths[1], "content").get_resource();
	});
	taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		normal = resourceManager->convert_to_aares<ecore::Texture2D>(paths[2], "content").get_resource();
	});
	taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		roughness = resourceManager->convert_to_aares<ecore::Texture2D>(paths[3], "content").get_resource();
	});
	taskComposer->execute(taskGroup, [&](tasks::TaskExecutionInfo)
	{
		metallic = resourceManager->convert_to_aares<ecore::Texture2D>(paths[4], "content").get_resource();
	});
	taskComposer->wait(taskGroup);
	
	resource::FirstCreationContext<ecore::OpaquePBRMaterial> creationContext;
	creationContext.materialName = "DefaultOpaqueMaterial";
	creationContext.materialPath = projectRootPath + "/content";
	creationContext.materialSettings.metallicTextureUUID = metallic->get_uuid();
	creationContext.materialSettings.baseColorTextureUUID = albedo->get_uuid();
	creationContext.materialSettings.roughnessTextureUUID = roughness->get_uuid();
	creationContext.materialSettings.normalTextureUUID = normal->get_uuid();
	creationContext.materialSettings.ambientOcclusionTextureUUID = ao->get_uuid();
	auto material = resourceManager->create_new_resource(creationContext).get_resource();
	_projectSettings->get_subsettings<ecore::RendererSubsettings>()->set_default_material_uuid(material->get_uuid());
}

UUID Project::get_default_material_uuid()
{
	return _projectSettings->get_subsettings<ecore::RendererSubsettings>()->get_default_material_uuid();
}
