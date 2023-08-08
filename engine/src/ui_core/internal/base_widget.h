#pragma once

#include <string>

namespace ad_astris::uicore::internal
{
	class BaseWidget
	{
		public:
			virtual ~BaseWidget() { }
			BaseWidget(const std::string& name) : _name(name) { }
		
			virtual bool draw() = 0;
			std::string get_name()
			{
				return _name;
			}

		protected:
			std::string _name;
	};
}