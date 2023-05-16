#include "sdl_window.h"
#include "profiler/logger.h"
#include "SDL.h"
#include <imgui_impl_sdl2.h>

using namespace ad_astris;

void sdl::SDLWindow::init(const WindowSize& windowSize)
{
	_windowSize.width = windowSize.width;
	_windowSize.height = windowSize.height;
	
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	_window = SDL_CreateWindow(
		"AdAstris Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowSize.width,
		_windowSize.height,
		window_flags
	);

	SDL_SetWindowResizable(_window, SDL_TRUE);

// #ifdef _WIN32	
// 	SDL_GetWindowWMInfo(_window, &_win32Info);
// #endif

	SDL_Delay(100);
}

void sdl::SDLWindow::handle_action_per_frame(Camera* camera, bool* bQuit)
{
	const Uint64 currentTicks{SDL_GetPerformanceCounter()};
	const Uint64 delta{ currentTicks - _previousTicks };
	_previousTicks = currentTicks;
	_elapsedSeconds = static_cast<float>(delta) / static_cast<float>(SDL_GetPerformanceFrequency());

	_cameraMovement = false;

	handle_events(bQuit);

	SDL_PumpEvents();

	handle_camera_action(camera);
}

void sdl::SDLWindow::handle_camera_action(Camera* camera)
{
	const Uint8 *state = SDL_GetKeyboardState(nullptr);
	int x, y;
	Uint32 mouseState = SDL_GetMouseState(&x, &y);
	if (SDL_BUTTON(3) & mouseState)
	{
		if (!_enabledRelMode)
		{
			_enabledRelMode = SDL_TRUE;
			SDL_SetRelativeMouseMode(_enabledRelMode);
		}
	
		if (state[SDL_SCANCODE_W])
			camera->process_keyboard(FORWARD, _elapsedSeconds);
		if (state[SDL_SCANCODE_S])
			camera->process_keyboard(BACKWARD, _elapsedSeconds);
		if (state[SDL_SCANCODE_A])
			camera->process_keyboard(LEFT, _elapsedSeconds);
		if (state[SDL_SCANCODE_D])
			camera->process_keyboard(RIGHT, _elapsedSeconds);

		if (_cameraMovement)
		{
			float xPos, yPos;

			xPos = static_cast<float>(_mousePosition.mouseX);
			yPos = static_cast<float>(_mousePosition.mouseY);
			if (_firstMouse)
			{
				xPos = 0.0f;
				yPos = 0.0f;
				_firstMouse = false;
			}
			
			camera->process_mouse_movement(xPos, yPos);
		}
	}
	else
	{
		if (_enabledRelMode)
		{
			_enabledRelMode = SDL_FALSE;
			SDL_SetRelativeMouseMode(_enabledRelMode);
			SDL_WarpMouseInWindow(_window, _windowSize.width / 2, _windowSize.height / 2);
		}
	}
}

void sdl::SDLWindow::handle_events(bool* bQuit)
{
	//Handle events on queue
	while (SDL_PollEvent(&_events) != 0)
	{
		SDL_PumpEvents();
		ImGui_ImplSDL2_ProcessEvent(&_events);

		//close the window when user alt-f4s or clicks the X button
		if (_events.type == SDL_QUIT)
		{
			*bQuit = true;
		}
		if (_events.type == SDL_KEYDOWN)
		{
			SDL_PumpEvents();
			if (_events.key.keysym.sym == SDLK_ESCAPE)
			{
				*bQuit = true;
			}
		}

		if (_events.type == SDL_MOUSEMOTION)
		{
			_cameraMovement = true;
			SDL_PumpEvents();
			_mousePosition.mouseX = _events.motion.xrel;
			_mousePosition.mouseY = _events.motion.yrel;
		}
	}
}
