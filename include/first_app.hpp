#pragma once

#include "rendering/lve_window.hpp"
#include "rendering/lve_pipeline.hpp"
#include "rendering/lve_device.hpp"
#include "rendering/lve_model.hpp"
#include "rendering/lve_renderer.hpp"
#include "rendering/lve_game_object.hpp"
#include "rendering/lve_frame_info.hpp"
#include "rendering/lve_camera.hpp"
#include "rendering/lve_descriptors.hpp"

#include <memory>
#include <vector>

#define MAX_LIGHTS 10


namespace lve {
	
	struct PointLight {
		glm::vec3 position{};
		alignas(16) glm::vec4 color{};  // w is intensity
	};

	struct GlobalUBO { // See https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/app09lev1sec2.html for alignment rules
		glm::mat4 projectionMatrix{1.f};
		glm::mat4 cameraMatrix{1.f};
		glm::mat4 invCameraMatrix{1.f};
		glm::vec3 ambientLightColor{0.02f};
		
		int numLights;
		PointLight lights[MAX_LIGHTS];

	};
	class FirstApp {
		public:
			static constexpr int WIDTH = 900;
			static constexpr int HEIGHT = 900;

			FirstApp();
			~FirstApp();

			FirstApp(const FirstApp &) = delete;
			FirstApp& operator=(const FirstApp &) = delete;

			void run();
			void update(GlobalUBO &ubo, FrameInfo &frameInfo);

			void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
			void mousePosCallback(GLFWwindow* window, double x, double y);


		private:
			void loadGameObjects();

			LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
			LveDevice lveDevice{lveWindow};
			LveRenderer lveRenderer{lveWindow, lveDevice};

			std::unique_ptr<LveDescriptorPool> globalPool;
			LveGameObject::Map gameObjects;
	};
}