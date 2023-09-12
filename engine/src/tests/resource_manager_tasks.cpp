#include "resource_manager/resource_manager.h"
#include "multithreading/task_composer.h"
#include "engine_core/texture/texture2D.h"
#include "file_system/IO.h"
#include "core/timer.h"
#include <filesystem>

using namespace ad_astris;

resource::ResourceManager* RESOURCE_MANAGER{ nullptr };
tasks::TaskComposer* TASK_COMPOSER{ nullptr };
io::FileSystem* FILE_SYSTEM{ nullptr };

void test_texture_loading()
{
	std::string texturesFolder = (FILE_SYSTEM->get_engine_root_path() + "/starter_content/textures").c_str();
	io::URI outputDirPath = FILE_SYSTEM->get_engine_root_path() + "/tests_output";
	
	Timer timer;
	for (auto& dirEntity : std::filesystem::directory_iterator(texturesFolder))
	{
		if (!dirEntity.is_directory())
		{
			io::URI texturePath = dirEntity.path().string().c_str();
			RESOURCE_MANAGER->convert_to_aares<ecore::Texture2D>(texturePath, outputDirPath);
		}
	}
	LOG_INFO("Loading textures in one thread time: {} ms", timer.elapsed_milliseconds())

	timer.record();
	tasks::TaskGroup taskGroup;
	for (auto& dirEntity : std::filesystem::directory_iterator(texturesFolder))
	{
		if (!dirEntity.is_directory())
		{
			TASK_COMPOSER->execute(taskGroup, [dirEntity, outputDirPath](tasks::TaskExecutionInfo)
			{
				io::URI texturePath = dirEntity.path().string().c_str();
				RESOURCE_MANAGER->convert_to_aares<ecore::Texture2D>(texturePath, outputDirPath);
			});
		}
	}
	TASK_COMPOSER->wait(taskGroup);
	LOG_INFO("Loading textures in several threads time: {} ms", timer.elapsed_milliseconds())
}

int main()
{
	std::string engineRootPath = std::filesystem::current_path().string();
	engineRootPath.erase(engineRootPath.find("\\bin\\tests"), 10);
	FILE_SYSTEM = new io::EngineFileSystem(engineRootPath.c_str());
	FILE_SYSTEM->set_project_root_path("C:\\Users\\zaver\\AdAstrisProjects\\test2");

	events::EventManager* eventManager = new events::EventManager();

	RESOURCE_MANAGER = new resource::ResourceManager(FILE_SYSTEM, eventManager);
	TASK_COMPOSER = new tasks::TaskComposer();
	
	test_texture_loading();

	delete RESOURCE_MANAGER;
	delete TASK_COMPOSER;
	delete FILE_SYSTEM;
}
