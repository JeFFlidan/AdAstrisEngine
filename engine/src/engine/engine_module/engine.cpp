#include "engine.h"
#include "engine_core/fwd.h"
#include "engine_core/material/general_material_template.h"

using namespace ad_astris::engine::impl;

void Engine::init(EngineInitializationContext& initializationContext)
{
	_fileSystem = initializationContext.fileSystem;
	_moduleManager = initializationContext.moduleManager;
	_eventManager = initializationContext.eventManager;
	_mainWindow = initializationContext.mainWindow;

	_taskComposer = std::make_unique<tasks::TaskComposer>();
	LOG_INFO("Engine::init(): Initialized task composer")
	
	_resourceManager = std::make_unique<resource::ResourceManager>(_fileSystem, _eventManager);
	LOG_INFO("Engine::init(): Initialized resource manager")

	_world = std::make_unique<ecore::World>();
	LOG_INFO("Engine::init(): Initialized world")

	_systemManager = std::make_unique<ecs::SystemManager>();
	_systemManager->init();
	LOG_INFO("Engine::init(): Initialized system manager")
	_systemManager->generate_execution_order();
	_systemManager->add_entity_manager(_world->get_entity_manager());
	LOG_INFO("Engine::init(): Generated execution order")

	switch (initializationContext.projectInfo->newProjectTemplate)
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

	//auto rendererModule = _moduleManager->load_module<renderer::IRendererModule>("Renderer");
	//_renderer = rendererModule->get_renderer();
	//_renderer->init(_moduleManager, _resourceManager.get(), window, *_engineSettings.get());

	LOG_INFO("Engine::init(): Engine initialization completed")
}

void Engine::execute()
{
	
}

void Engine::save_and_cleanup(bool needToSave)
{
	_systemManager->cleanup();
	if (needToSave)
	{
		_resourceManager->save_resources();
		_projectSettings->serialize(_fileSystem);
	}
}

void Engine::create_new_blank_project()
{
	io::URI path = _fileSystem->get_project_root_path() + "/content/levels/base_level.aalevel";
	ecore::LevelHandle level = _resourceManager->create_level(path);
	_world->add_level(level.get_resource(), true, true);
	LOG_INFO("Engine::init(): Created new level")

	ecore::DefaultSettingsContext<ecore::ProjectSettings> defaultSettingsContext;
	io::URI relativePathToDefaultLevel = io::Utils::get_relative_path_to_file(_fileSystem->get_project_root_path(), path);
	io::Utils::replace_back_slash_to_forward(relativePathToDefaultLevel);
	defaultSettingsContext.defaultLevelPath = relativePathToDefaultLevel;
	io::URI aaprojectFilePath = io::Utils::find_file_with_specific_extension(_fileSystem->get_project_root_path(), ".aaproject");
	defaultSettingsContext.projectName = io::Utils::get_file_name(aaprojectFilePath);
	_projectSettings = std::make_unique<ecore::ProjectSettings>();
	_projectSettings->setup_default_settings(defaultSettingsContext);
	_projectSettings->deserialize(_fileSystem->get_project_root_path() + "/configs/project_settings.ini");
	_projectSettings->serialize(_fileSystem);
	LOG_INFO("BEFORE CREATE MATERIAL TEMPLATES")
	create_material_templates();
	LOG_INFO("Engine::init(): Saved project with default settings")
}

void Engine::load_existing_project()
{
	_projectSettings = std::make_unique<ecore::ProjectSettings>();
	_projectSettings->deserialize(_fileSystem->get_project_root_path() + "/configs/project_settings.ini");
	LOG_INFO("Engine::init(): Loaded project settings config")
	auto projectDescriptionSubsettings = _projectSettings->get_subsettings<ecore::ProjectDescriptionSubsettings>();
	LOG_INFO("After loading project description ")
	io::URI levelRelativePath = projectDescriptionSubsettings->get_default_level_path().c_str();
	io::URI levelPath = io::Utils::get_absolute_path_to_file(_fileSystem->get_project_root_path(), levelRelativePath);
	LOG_INFO("Level path: {}", levelPath.c_str())
	ecore::LevelHandle level = _resourceManager->load_level(levelPath);
	LOG_INFO("After loading level")
	_world->add_level(level.get_resource(), true, true);
	level.get_resource()->build_entities();
	LOG_INFO("After building entities")
	LOG_INFO("Engine::init(): Loaded default level {}", level.get_resource()->get_name()->get_full_name())
}

void Engine::create_material_templates()
{
	using namespace ecore::material;
	
	ShaderPassCreateInfo gbufferShaderPassCreateInfo;
	gbufferShaderPassCreateInfo.vertexShaderPath = "shaders/deferred/GBuffer.vert";
	gbufferShaderPassCreateInfo.fragmentShaderPath = "shaders/deferred/GBuffer.frag";
	gbufferShaderPassCreateInfo.passType = ShaderPassType::GBUFFER;
	
	ShaderPassCreateInfo deferredShaderPassCreateInfo;
	deferredShaderPassCreateInfo.vertexShaderPath = "shaders/deferred/deferred_lighting.vert";
	deferredShaderPassCreateInfo.fragmentShaderPath = "shaders/deferred/deferred_lighting.frag";
	deferredShaderPassCreateInfo.passType = ShaderPassType::DEFERRED_LIGHTING;

	ShaderPassCreateInfo oitPrepassShaderPassCreateInfo;
	oitPrepassShaderPassCreateInfo.vertexShaderPath = "shaders/oit/oit_geometry.vert";
	oitPrepassShaderPassCreateInfo.fragmentShaderPath = "shaders/oit/oit_geometry.frag";
	oitPrepassShaderPassCreateInfo.passType = ShaderPassType::OIT_PREPASS;

	ShaderPassCreateInfo oitShaderPassCreateInfo;
	oitShaderPassCreateInfo.vertexShaderPath = "shaders/oit/transparency.vert";
	oitShaderPassCreateInfo.fragmentShaderPath = "shaders/oit/transparency.frag";
	oitShaderPassCreateInfo.passType = ShaderPassType::OIT;
	
	ShaderPassCreateInfo taaShaderPassCreateInfo;
	taaShaderPassCreateInfo.vertexShaderPath = "shaders/common_shaders/quad.vert";
	taaShaderPassCreateInfo.fragmentShaderPath = "shaders/anti_aliasing/taa.frag";
	taaShaderPassCreateInfo.passType = ShaderPassType::TAA;
	
	ShaderPassCreateInfo directionalLightShaderPassCreateInfo;
	directionalLightShaderPassCreateInfo.vertexShaderPath = "shaders/shadow_mapping/dir_light_depth_map.vert";
	directionalLightShaderPassCreateInfo.fragmentShaderPath = "shaders/shadow_mapping/depth_map.frag";
	directionalLightShaderPassCreateInfo.passType = ShaderPassType::DIRECTIONAL_LIGHT_SHADOWS;
	
	ShaderPassCreateInfo pointLightShaderPassCreateInfo;
	pointLightShaderPassCreateInfo.vertexShaderPath = "shaders/shadow_mapping/point_light_depth_map.vert";
	pointLightShaderPassCreateInfo.fragmentShaderPath = "shaders/shadow_mapping/point_light_depth_map.frag";
	pointLightShaderPassCreateInfo.passType = ShaderPassType::POINT_LIGHT_SHADOWS;
	
	ShaderPassCreateInfo spotLightShaderPassCreateInfo;
	spotLightShaderPassCreateInfo.vertexShaderPath = "shaders/shadow_mapping/spot_light_depth_map.vert";
	spotLightShaderPassCreateInfo.fragmentShaderPath = "shaders/shadow_mapping/depth_map.frag";
	spotLightShaderPassCreateInfo.passType = ShaderPassType::SPOT_LIGHT_SHADOWS;

	ShaderPassCreateInfo compositeShaderPassCreateInfo;
	compositeShaderPassCreateInfo.vertexShaderPath = "shaders/common_shaders/quad.vert";
	compositeShaderPassCreateInfo.fragmentShaderPath = "shaders/postprocessing/composite.frag";
	compositeShaderPassCreateInfo.passType = ShaderPassType::COMPOSITE;

	ShaderPassCreateInfo postprocessingShaderPassCreateInfo;
	postprocessingShaderPassCreateInfo.vertexShaderPath = "shaders/common_shaders/quad.vert";
	postprocessingShaderPassCreateInfo.fragmentShaderPath = "shaders/postprocessing/postprocessing.frag";
	postprocessingShaderPassCreateInfo.passType = ShaderPassType::POSTPROCESSING;

	ShaderPassCreateInfo cullingShaderPassCreateInfo;
	cullingShaderPassCreateInfo.computeShader = "shaders/compute/draw_cull.comp";
	cullingShaderPassCreateInfo.passType = ShaderPassType::CULLING;

	ShaderPassCreateInfo reduceDepthShaderPassCreateInfo;
	reduceDepthShaderPassCreateInfo.computeShader = "shaders/compute/reduce_depth.comp";
	reduceDepthShaderPassCreateInfo.passType = ShaderPassType::REDUCE_DEPTH;

	resource::FirstCreationContext<ecore::GeneralMaterialTemplate> opaquePBRTemplate;
	opaquePBRTemplate.shaderPassCreateInfos = {
		directionalLightShaderPassCreateInfo,
		pointLightShaderPassCreateInfo,
		spotLightShaderPassCreateInfo,
		gbufferShaderPassCreateInfo,
		deferredShaderPassCreateInfo
	};
	opaquePBRTemplate.materialTemplateName = "OpaquePBRMaterialTemplate";

	resource::FirstCreationContext<ecore::GeneralMaterialTemplate> oitTemplate;
	oitTemplate.shaderPassCreateInfos = {
		oitPrepassShaderPassCreateInfo,
		oitShaderPassCreateInfo
	};
	oitTemplate.materialTemplateName = "OITMaterialTemplate";

	resource::FirstCreationContext<ecore::GeneralMaterialTemplate> taaTemplate;
	taaTemplate.shaderPassCreateInfos = {
		taaShaderPassCreateInfo
	};
	taaTemplate.materialTemplateName = "TAAMaterialTemplate";

	resource::FirstCreationContext<ecore::GeneralMaterialTemplate> compositeTemplate;
	compositeTemplate.shaderPassCreateInfos = {
		compositeShaderPassCreateInfo
	};
	compositeTemplate.materialTemplateName = "CompositeMaterialTemplate";

	resource::FirstCreationContext<ecore::GeneralMaterialTemplate> postprocessingTemplate;
	postprocessingTemplate.shaderPassCreateInfos = {
		postprocessingShaderPassCreateInfo
	};
	postprocessingTemplate.materialTemplateName = "PostprocessingMaterialTemplate";

	resource::FirstCreationContext<ecore::GeneralMaterialTemplate> cullingTemplate;
	cullingTemplate.shaderPassCreateInfos = {
		cullingShaderPassCreateInfo
	};
	cullingTemplate.materialTemplateName = "CullingMaterialTemplate";

	resource::FirstCreationContext<ecore::GeneralMaterialTemplate> reduceDepthTemplate;
	reduceDepthTemplate.shaderPassCreateInfos = {
		reduceDepthShaderPassCreateInfo
	};
	reduceDepthTemplate.materialTemplateName = "ReduceDepthMaterialTemplate";
	
	_resourceManager->create_new_resource(opaquePBRTemplate);
	_resourceManager->create_new_resource(oitTemplate);
	_resourceManager->create_new_resource(taaTemplate);
	_resourceManager->create_new_resource(compositeTemplate);
	_resourceManager->create_new_resource(postprocessingTemplate);
	_resourceManager->create_new_resource(cullingTemplate);
	_resourceManager->create_new_resource(reduceDepthTemplate);
}
