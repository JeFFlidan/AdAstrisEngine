#pragma once

#include "application_core/window.h"

#include <glad/glad.h>
#include <glfw3.h>

namespace ad_astris::acore
{
	class GlfwWindow : public IWindow
	{
		public:
			GlfwWindow(WindowCreationContext& creationContext);
			void cleanup();
			virtual bool process_messages() override;
			void swap_buffers();
			void get_window_size(int* width, int* height);
		
			GLFWwindow* get_window_handle()
			{
				return _window;
			}
		
			virtual HWND get_hWnd() override;

		private:
			GLFWwindow* _window{ nullptr };
	};
}