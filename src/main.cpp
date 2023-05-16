#include <vulkan_renderer/vk_renderer.h>
#include <chrono>
#include <iostream>
#include <profiler/logger.h>

int main(int argc, char* argv[])
{
	ad_astris::VkRenderer engine;

	auto start = std::chrono::system_clock::now();
	engine.init();
	auto end = std::chrono::system_clock::now();
	auto diff = end - start;
	LOG_INFO("Init engine took {} {}", std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0, "ms");
	
	engine.run();	

	engine.cleanup();

	return 0;
}
