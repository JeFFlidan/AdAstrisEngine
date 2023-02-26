#include "uuid.h"
#include "profiler/logger.h"
#include <random>

using namespace ad_astris;

static std::random_device sRandomDevice;
static std::mt19937_64 sRandomEngine(sRandomDevice());
static std::uniform_int_distribution<uint64_t> sUniformDistribution;

UUID::UUID() : _uuid(sUniformDistribution(sRandomEngine))
{
	LOG_INFO("UUID {}", _uuid)
}

UUID::UUID(uint64_t uuid) : _uuid(uuid)
{
	
}

