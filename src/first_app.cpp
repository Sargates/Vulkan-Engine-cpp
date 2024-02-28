#include "first_app.hpp"
#include <iostream>

namespace lve {
	void FirstApp::run() {
		while (! lveWindow.shouldClose()) {
			glfwPollEvents();
			
		}
	}
}