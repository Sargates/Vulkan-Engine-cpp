#include "lve_window.hpp"
#include <stdexcept>

namespace lve {
	LveWindow::LveWindow(int w, int h, std::string name) : width(w), height(h), windowName(name) {
		initWindow();
	}

	LveWindow::~LveWindow() {
		glfwDestroyWindow(_window);
		glfwTerminate();
	}


	void LveWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		_window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(_window, this);
		glfwSetFramebufferSizeCallback(_window, frameBufferResizedCallback);
	}

	
	void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, _window, nullptr, surface) != VK_SUCCESS)
			throw std::runtime_error("Failed to create window surface");
	}

	void LveWindow::frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto lveWindow = reinterpret_cast<LveWindow*>(glfwGetWindowUserPointer(window));
		lveWindow->frameBufferResized = true;
		lveWindow->width = width;
		lveWindow->height = height;
	}
	
}