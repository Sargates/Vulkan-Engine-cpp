#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>

namespace lve {
	class LveWindow {
		public:
			LveWindow(int w, int h, std::string name);
			~LveWindow();
			bool shouldClose() { return glfwWindowShouldClose(window); }

			LveWindow(const LveWindow &) = delete;
			LveWindow &operator=(const LveWindow &) = delete;
		
		private:
			void initWindow();
			const int width, height;
			std::string windowName;
		
			GLFWwindow* window;
	};
} // namespace lve