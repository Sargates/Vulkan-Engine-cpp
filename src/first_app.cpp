#include "simple_render_system.hpp"
#include "first_app.hpp"


#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>
#include <cmath>


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
		// std::cout << "maxPushConstantSize = " << lveDevice.properties.limits.maxPushConstantsSize << std::endl;
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
		// glfwSetKeyCallback(lveWindow.getWindow(), keyCallback);


		while (! lveWindow.shouldClose()) {
			glfwPollEvents();
			if (auto commandBuffer = lveRenderer.beginFrame()) {

				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
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

		glm::vec3 colors[5] = {
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};

		std::shared_ptr<LveModel> lveModel = createCubeModel(lveDevice, glm::vec3{0.f});

		LveGameObject cube = LveGameObject::createGameObject();
		cube.model = lveModel;
		cube.transform.translation = {.0f, .0f, .5f};
		cube.transform.scale = {.5f, .5f, .5f};
		gameObjects.push_back(std::move(cube));
	}
	
}