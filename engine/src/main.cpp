#include <application/ad_astris_engine.h>

int main(int argc, char** argv)
{
	auto adAstrisEngine = std::make_unique<ad_astris::app::AdAstrisEngine>();
	if (adAstrisEngine->init())
	{
		adAstrisEngine->execute();
		adAstrisEngine->save_and_cleanup();
	}

	return 0;
}
