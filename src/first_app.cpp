#include "simple_render_system.hpp"
#include "first_app.hpp"


#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>
#include <cmath>

//! Sierpinski's Triangle Generator
	void sierpinski(
			std::vector<lve::LveModel::Vertex> &vertices,
			int depth,
			glm::vec2 left,
			glm::vec2 right,
			glm::vec2 top) {
		if (depth > 0) {
			auto leftTop = 0.5f * (left + top);
			auto rightTop = 0.5f * (right + top);
			auto leftRight = 0.5f * (left + right);
			sierpinski(vertices, depth - 1, left, leftRight, leftTop);
			sierpinski(vertices, depth - 1, leftRight, right, rightTop);
			sierpinski(vertices, depth - 1, leftTop, rightTop, top);
			return;
		}
		vertices.push_back({top, 	{(top.x+1)/2.0f, (top.y+1)/2.0f, 0.0f}});
		vertices.push_back({right, 	{(top.x+1)/2.0f, (top.y+1)/2.0f, 0.0f}});
		vertices.push_back({left, 	{(top.x+1)/2.0f, (top.y+1)/2.0f, 0.0f}});
	}

namespace lve {

	FirstApp::FirstApp() {
		loadGameObjects();
	}
	FirstApp::~FirstApp() {}
	void FirstApp::run() {
		// std::cout << "maxPushConstantSize = " << lveDevice.properties.limits.maxPushConstantsSize << std::endl;
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};


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
		std::vector<LveModel::Vertex> vertices {
			{{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
		};

		// std::vector<LveModel::Vertex> vertices {
		// 	{{ -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f }}, // Bottom Left	-- Black
		// 	{{ -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }}, // Top Left		-- Red
		// 	{{  1.0f, -1.0f }, { 1.0f, 1.0f, 0.0f }}, // Top Right		-- Yellow
		// 	{{ -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f }}, // Bottom Left	-- Black
		// 	{{  1.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }}, // Bottom Right	-- Green
		// 	{{  1.0f, -1.0f }, { 1.0f, 1.0f, 0.0f }}, // Top Right		-- Yellow
		// };
		// std::vector<LveModel::Vertex> vertices{};
		// sierpinski(vertices, 7, glm::vec2{-0.9f, 0.9f}, glm::vec2{0.9f, 0.9f}, glm::vec2{0.0f, -0.9f});

		glm::vec3 colors[5] = {
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};

		auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

		for (int i=0; i<20; i++) {
			auto triangle = LveGameObject::createGameObject();
			triangle.model = lveModel;
			triangle.color = colors[i%5];
			triangle.transform2D.scale = glm::vec2{0.4f + (1.8f-0.4f) * (i/20.f)};

			gameObjects.push_back(std::move(triangle));
		}
	}
	
}