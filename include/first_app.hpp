#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_model.hpp"
#include "lve_renderer.hpp"
#include "lve_game_object.hpp"
#include "lve_camera.hpp"

#include <memory>
#include <vector>

namespace lve {
	class FirstApp {
		public:
			static constexpr int WIDTH = 900;
			static constexpr int HEIGHT = 900;

			FirstApp();
			~FirstApp();

			FirstApp(const FirstApp &) = delete;
			FirstApp& operator=(const FirstApp &) = delete;

			void run();
			// This must be static or doesn't fit the signature of `GLFWkeyfun`
			void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
			void mousePosCallback(GLFWwindow* window, double x, double y);


		private:
			void loadGameObjects();

			LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
			LveDevice lveDevice{lveWindow};
			LveRenderer lveRenderer{lveWindow, lveDevice};

			std::vector<LveGameObject> gameObjects;
	};
}