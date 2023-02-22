#pragma once

#include "actors_properties_ui.h"

namespace ad_astris::ui
{
	class PropertiesWindow : public IWindow
	{
		public:
			PropertiesWindow() = default;
			~PropertiesWindow() final = default;

			void draw_window(void* data) final;

		private:
			// temporary solution until object picking is implemented
			bool _isPoint{ true };
			bool _isDir{ false };
			bool _isSpot{ false };
			ActorType _actorType{ None };
			void* _actorData{ nullptr };
		
			void draw_ui(void* data) final;
	};
}
