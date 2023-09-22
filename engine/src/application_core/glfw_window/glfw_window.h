#pragma once

#include "application_core/window.h"

#include <glad/glad/glad.h>
#include <glfw/glfw3.h>

namespace ad_astris::acore
{
	class GlfwWindow : public IWindow
	{
		public:
			GlfwWindow(WindowCreationContext& creationContext);
			void cleanup();
			virtual bool process_messages() override;
			virtual void close() override { }
			void swap_buffers();
			void get_window_size(int* width, int* height);
		
			GLFWwindow* get_window_handle()
			{
				return _window;
			}
		
			virtual HWND get_hWnd() override;
			virtual uint32_t get_width() override { return 0; }
			virtual uint32_t get_height() override { return 0; }
			virtual bool is_running() override { return true; }

		private:
			GLFWwindow* _window{ nullptr };
	};
}