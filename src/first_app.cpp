#include "first_app.hpp"

#include "lve_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"


#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>
#include <vector>


namespace lve {
	
	FirstApp::FirstApp() {
		loadGameObjects();
	}
	FirstApp::~FirstApp() {}

	std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
		std::vector<LveModel::Vertex> vertices{
		
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
		
			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
		
			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
		
			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
		
			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
		
			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
		
		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<LveModel>(device, vertices);
	}

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
		// glfwSetKeyCallback(lveWindow.getWindow(), keyCallback);
		LveCamera camera{};
		camera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});

		auto viewerObject = LveGameObject::createGameObject();
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();


		while (! lveWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.moveInPlaneXZ(lveWindow.getWindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);


			float aspectRatio = lveRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(90.f), aspectRatio, 0.1, 10.f);
			

			if (auto commandBuffer = lveRenderer.beginFrame()) {
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
			if (glfwGetKey(lveWindow.getWindow(), GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(lveWindow.getWindow(), GLFW_TRUE); }
			// if (glfwGetKey(lveWindow.getWindow(), GLFW_KEY_W)) { gameObjects[0].transform2D.rotation += 0.01; }
			// if (glfwGetKey(lveWindow.getWindow(), GLFW_KEY_A)) { gameObjects[0].transform2D.scale.x += 0.005f; }
			// if (glfwGetKey(lveWindow.getWindow(), GLFW_KEY_S)) { gameObjects[0].transform2D.rotation -= 0.01; }
			// if (glfwGetKey(lveWindow.getWindow(), GLFW_KEY_D)) { gameObjects[0].transform2D.scale.x -= 0.005f; }
		}

		vkDeviceWaitIdle(lveDevice.device());
	}
	void FirstApp::loadGameObjects() {

		// std::vector<LveModel::Vertex> vertices{};
		// sierpinski(vertices, 7, glm::vec2{-0.9f, 0.9f}, glm::vec2{0.9f, 0.9f}, glm::vec2{0.0f, -0.9f});

		std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, glm::vec3{0.f});

		LveGameObject cube = LveGameObject::createGameObject();
		cube.model = lveModel;
		cube.transform.translation = {.0f, .0f, 2.5f};
		cube.transform.scale = {.5f, .5f, .5f};
		gameObjects.push_back(std::move(cube));
	}
	
}