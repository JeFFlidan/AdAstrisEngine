#pragma once

#include "api.h"
#include <cstdint>

namespace ad_astris::ecs
{
	template<typename T>
	class IdGenerator
	{
		public:
			template<typename U>
			static uint32_t generate_id()
			{
				static const uint32_t id{ ++_id };
				return id;
			}

			template<typename U>
			static uint32_t get()
			{
				return _id;
			}

		private:
			static uint32_t _id;
	};

	template<typename T>
	uint32_t IdGenerator<T>::_id{ 0 };
}