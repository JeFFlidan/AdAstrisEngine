#pragma once
#include <stdint.h>
#include <functional>

namespace ad_astris
{
	class UUID
	{
		public:
			UUID();
			UUID(uint64_t uuid);

			operator uint64_t() const { return _uuid; }
			bool operator==(const UUID& uuid) const
			{
				return _uuid == uuid._uuid;
			}
		private:
			uint64_t _uuid;
	};
}

namespace std
{
	template<>
	struct hash<ad_astris::UUID>
	{
		std::size_t operator()(const ad_astris::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}
