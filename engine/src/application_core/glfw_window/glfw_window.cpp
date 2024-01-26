#include "profiler/logger.h"
#include "glfw_window.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#include <windows.h>

using namespace ad_astris;
using namespace acore;

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GlfwWindow::GlfwWindow(WindowCreationContext& creationContext)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	_window = glfwCreateWindow(creationContext.width, creationContext.height, creationContext.windowTitle.c_str(), nullptr, nullptr);
	if (_window == nullptr)
	{
		glfwTerminate();
		throw std::runtime_error("failed to create window");
	}

	glfwMakeContextCurrent(_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		LOG_FATAL("GlfwWindow::GlfwWindow(): Failed to load glad")

	glfwSetFramebufferSizeCallback(_window, glfw_framebuffer_size_callback);

	glfwSetWindowAttrib(_window, GLFW_RESIZABLE, creationContext.isResizable);
	HWND hWnd = glfwGetWin32Window(_window);
	SetWindowLongPtr(hWnd, GWL_STYLE, GetWindowLongPtrA(hWnd, GWL_STYLE) & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX));

}

void GlfwWindow::cleanup()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

bool GlfwWindow::process_messages()
{
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(_window, true);

	return glfwWindowShouldClose(_window);
}

void GlfwWindow::swap_buffers()
{
	glfwSwapBuffers(_window);
	glfwPollEvents();
}

void GlfwWindow::get_window_size(int* width, int* height)
{
	glfwGetWindowSize(_window, width, height);
}

HWND GlfwWindow::get_hWnd()
{
	return HWND();
}
