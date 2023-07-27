#pragma once

namespace ad_astris::acore
{
	class IWindow
	{
		public:
			virtual ~IWindow() { }
			virtual bool process_messages() = 0;
	};
}