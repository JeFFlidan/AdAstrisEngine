#include <vk_engine.h>
#include <chrono>
#include <iostream>
#include <logger.h>

int main(int argc, char* argv[])
{
	VulkanEngine engine;

	auto start = std::chrono::system_clock::now();
	engine.init();
	auto end = std::chrono::system_clock::now();
	auto diff = end - start;
	LOG_INFO("Init engine took {} {}", std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0, "ms");
	
	engine.run();	

	engine.cleanup();

	return 0;
}
