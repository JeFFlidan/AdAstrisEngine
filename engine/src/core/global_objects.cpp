#include "global_objects.h"
#include "file_system/IO.h"

using namespace ad_astris;

void GlobalObjects::init_file_system()
{
	std::string _projectPath = std::filesystem::current_path().string();
	_projectPath.erase(_projectPath.find("\\bin"), 4);
	_globalObjectContext->fileSystem = std::make_unique<io::EngineFileSystem>(_projectPath.c_str());
}

void GlobalObjects::init_module_manager()
{
	_globalObjectContext->moduleManager = std::make_unique<ModuleManager>(FILE_SYSTEM());
}

void GlobalObjects::init_task_composer(uint32_t threadCount)
{
	_globalObjectContext->taskComposer = std::make_unique<tasks::TaskComposer>(threadCount);
}

void GlobalObjects::init_event_manager()
{
	_globalObjectContext->eventManager = std::make_unique<events::EventManager>();
}

void GlobalObjects::init_resource_manager()
{
	_globalObjectContext->resourceManager = std::make_unique<resource::ResourceManager>(FILE_SYSTEM(), EVENT_MANAGER());
}

void GlobalObjects::init_world()
{
	_globalObjectContext->ecsTypeInfoTable = std::make_unique<ecs::TypeInfoTable>();
	ecs::set_type_info_table(ECS_TYPE_INFO_TABLE());
	ecore::WorldCreationContext worldCreationContext;
	worldCreationContext.eventManager = EVENT_MANAGER();
	worldCreationContext.taskComposer = TASK_COMPOSER();
	_globalObjectContext->world = std::make_unique<ecore::World>(worldCreationContext);
	_globalObjectContext->ecsUIManager = std::make_unique<uicore::ECSUiManager>(WORLD()->get_entity_manager());
}

void GlobalObjects::init_system_manager()
{
	ecs::EngineManagers managers;
	managers.eventManager = EVENT_MANAGER();
	managers.resourceManager = RESOURCE_MANAGER();
	managers.taskComposer = TASK_COMPOSER();
	managers.entityManager = WORLD()->get_entity_manager();
	_globalObjectContext->systemManager = std::make_unique<ecs::SystemManager>();
	SYSTEM_MANAGER()->init(managers);
}

void GlobalObjects::init_profiler_instance()
{
	profiler::ProfilerInstanceInitContext initContext{};
	_globalObjectContext->profilerInstance = std::make_unique<profiler::ProfilerInstance>(initContext);
}
