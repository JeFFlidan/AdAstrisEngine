#pragma once

//For now I've added SDL support for Windows but I'll think is it a good idea after implementing win32 window 

#include "engine/camera.h"

#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_stdinc.h>
#include <SDL_video.h>

// #ifdef _WIN32
// 	#include <SDL_syswm.h>
// #endif

namespace ad_astris::sdl 
{
	class SDLWindow
	{
		public:
			struct WindowSize
			{
				int32_t width{ 0 };
				int32_t height{ 0 };
			};
		
			void init(const WindowSize& windowSize);
			void handle_action_per_frame(Camera* camera, bool* bQuit);
// #ifdef _WIN32
// 			SDL_SysWMinfo* get_win32_info() { return &_win32Info; }
// #endif
			SDL_Window* get_window() { return _window; }
			WindowSize get_window_size() { return _windowSize; }
		
		private:
			SDL_Window* _window{ nullptr };
			SDL_Event _events;
			SDL_bool _enabledRelMode{ SDL_FALSE };
			Uint64 _previousTicks{};
			float _elapsedSeconds{};
// #ifdef _WIN32
// 			SDL_SysWMinfo _win32Info;
// #endif
		
			WindowSize _windowSize;
			bool _firstMouse{ true };
			bool _cameraMovement{ false };
			struct
			{
				int32_t mouseX;
				int32_t mouseY;
			} _mousePosition;

			void handle_camera_action(Camera* camera);
			void handle_events(bool* bQuit);
	};
}