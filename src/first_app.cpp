#include "first_app.hpp"

#include "lve_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"
#include "math.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>
#include <vector>

#define print(x) std::cout << x << std::endl

glm::vec2 lastMousePos{-1.0};

lve::LveCamera* activeCamera = nullptr;

namespace lve {
	
	FirstApp::FirstApp() {
		loadGameObjects();
	}
	FirstApp::~FirstApp() {}

	std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
		LveModel::Builder modelBuilder{};
		modelBuilder.vertices = {
			{{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}}, // Black	- 0
			{{0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}}, // Blue		- 1
			{{0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}}, // Green	- 2
			{{0.f, 1.f, 1.f}, {0.f, 1.f, 1.f}}, // Cyan		- 3
			{{1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}}, // Red		- 4
			{{1.f, 0.f, 1.f}, {1.f, 0.f, 1.f}}, // Magenta	- 5
			{{1.f, 1.f, 0.f}, {1.f, 1.f, 0.f}}, // Yellow	- 6
			{{1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}}  // White	- 7
		};

		modelBuilder.indices = {
			0, 2, 4,   4, 2, 6,	// Front
			4, 6, 5,   5, 6, 7,	// Right
			5, 7, 1,   1, 7, 3,	// Back
			1, 3, 2,   1, 2, 0,	// Left
			1, 0, 5,   0, 4, 5,	// Top
			2, 3, 7,   2, 7, 6	// Bottom
		};

		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}
		return std::make_unique<LveModel>(device, modelBuilder);
	}

	std::unique_ptr<LveModel> createGrid(LveDevice& device, glm::vec3 offset, float size, glm::ivec2 dimensions) {
		LveModel::Builder modelBuilder{};

		for (int i=-dimensions.x/2.0f; i<dimensions.x/2.0f; i++) {
			for (int j=-dimensions.y/2.0f; j<dimensions.y/2.0f; j++) {
				glm::vec3 color = ((i+j) % 2 == 0) ? glm::vec3{0.7f, 0.7f, 0.7f} : glm::vec3{0.3f, 0.3f, 0.3f};
				LveModel::Vertex corner1{{i*size     , 0, j*size     }, color, {0.f, 1.f, 0.f}, {}};
				LveModel::Vertex corner2{{i*size+size, 0, j*size     }, color, {0.f, 1.f, 0.f}, {}};
				LveModel::Vertex corner3{{i*size     , 0, j*size+size}, color, {0.f, 1.f, 0.f}, {}};
				LveModel::Vertex corner4{{i*size+size, 0, j*size+size}, color, {0.f, 1.f, 0.f}, {}};
				modelBuilder.vertices.push_back(corner1);
				modelBuilder.vertices.push_back(corner2);
				modelBuilder.vertices.push_back(corner4);
				modelBuilder.vertices.push_back(corner1);
				modelBuilder.vertices.push_back(corner4);
				modelBuilder.vertices.push_back(corner3);
			}
		}
		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}
		return std::make_unique<LveModel>(device, modelBuilder);
	}

	void mouseCallback(GLFWwindow* window, double xd, double yd) {
		float x = static_cast<float>(xd); float y = static_cast<float>(yd);
		
		if (lastMousePos == glm::vec2{-1.0}) // If last pos is not set, just make delta = 0; as if nothing happens
			lastMousePos = glm::vec2{x, y};

		glm::vec2 currentPos{x, y};
		glm::vec2 delta = (currentPos-lastMousePos) / 900.0f;
		
		activeCamera->transform.rotation.x += delta.y; // Delta-Y maps to rotation about X-axis -- Negative because Vulkan
		activeCamera->transform.rotation.y += delta.x; // Delta-X maps to rotation about Y-axis

		float max = 0.01f + glm::half_pi<float>();
		float min = 0.01f + -glm::half_pi<float>();
		activeCamera->transform.rotation.x = std::clamp(activeCamera->transform.rotation.x, min, max);



		activeCamera->UpdateView();
		lastMousePos = currentPos;
	}
	// void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// 	if (key == GLFW_KEY_W) { activeCamera->transform.position += activeCamera->transform.forward; }
	// 	if (key == GLFW_KEY_S) { activeCamera->transform.position += activeCamera->transform.backward; }
	// 	if (key == GLFW_KEY_A) { activeCamera->transform.position += activeCamera->transform.left; }
	// 	if (key == GLFW_KEY_D) { activeCamera->transform.position += activeCamera->transform.right; }
	// 	if (key == GLFW_KEY_SPACE) { activeCamera->transform.position += activeCamera->transform.up; }
	// 	if (key == GLFW_KEY_LEFT_SHIFT) { activeCamera->transform.position += activeCamera->transform.down; }
	// 	activeCamera->UpdateView();
	// }
	void checkKeys(GLFWwindow* window, float dt) {
		if (glfwGetKey(window, GLFW_KEY_W))          { activeCamera->transform.position += dt * activeCamera->transform.forward; }
		if (glfwGetKey(window, GLFW_KEY_S))          { activeCamera->transform.position += dt * activeCamera->transform.backward; }
		if (glfwGetKey(window, GLFW_KEY_A))          { activeCamera->transform.position += dt * activeCamera->transform.left; }
		if (glfwGetKey(window, GLFW_KEY_D))          { activeCamera->transform.position += dt * activeCamera->transform.right; }
		if (glfwGetKey(window, GLFW_KEY_SPACE))      { activeCamera->transform.position += dt * activeCamera->transform.up; }
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) { activeCamera->transform.position += dt * activeCamera->transform.down; }
		activeCamera->UpdateView();
	}


	void FirstApp::run() {

		//! This fucking solution of needing to have a pointer you need to cast and dereference is the stupidest thing ever. Fair enough because C++ doesn't have (adequate) reflection.
		// glfwSetWindowUserPointer(lveWindow.getWindow(), this);
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};

		// Cameras
		LveCamera camera{};
		activeCamera = &camera; // Set `activeCamera` to this local camera
		KeyboardMovementController cameraController{};
		auto viewerObject = LveGameObject::createGameObject();
		// viewerObject.transform.position = {0, -1.f, -2.5f};
		camera.transform.position = {0, 1.f, -2.5f};
		// camera.setViewDirection(camera.transform.position, {0.f, -1.f, 0.f}, {0.f, 1.f, 0.f});
		camera.transform.rotation = glm::zero<glm::vec3>();

		// glfwSetKeyCallback(lveWindow.getWindow(), handleKey);
		glfwSetInputMode(lveWindow.window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(lveWindow.window(), mouseCallback);
		
		
		// Clock
		auto currentTime = std::chrono::high_resolution_clock::now();


		while (! lveWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;
			int FPS= (deltaTime <= 0.001f) ? 999 : 1/deltaTime;
			std::cout << FPS << "\r";



			checkKeys(lveWindow.window(), deltaTime);
			// cameraController.moveInPlaneXZ(lveWindow.getWindow(), frameTime, camera.transform);
			camera.UpdateView();

			float aspectRatio = lveRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(90.f), aspectRatio, 0.1, 100.f);
			

			if (auto commandBuffer = lveRenderer.beginFrame()) {
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
			if (glfwGetKey(lveWindow.window(), GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(lveWindow.window(), GLFW_TRUE); }
		}
		std::cout << std::endl;

		vkDeviceWaitIdle(lveDevice.device());
	}
	void FirstApp::loadGameObjects() {

		std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "./resources/smooth_vase.obj");
		LveGameObject gameObject = LveGameObject::createGameObject();
		gameObject.model = lveModel;
		gameObject.transform.position = {-1.f, 1.f, 0.f};
		gameObject.transform.scale = glm::vec3{3.f};
		gameObjects.push_back(std::move(gameObject));


		lveModel = LveModel::createModelFromFile(lveDevice, "./resources/colored_cube.obj");
		LveGameObject cube = LveGameObject::createGameObject();
		cube.model = lveModel;
		cube.transform.position = {1.f, 1.f, 0.f};
		cube.transform.scale = glm::vec3{0.5f};
		gameObjects.push_back(std::move(cube));

		lveModel = createGrid(lveDevice, glm::vec3{0.f}, 0.5, glm::ivec2{16});
		LveGameObject grid = LveGameObject::createGameObject();
		grid.model = lveModel;
		grid.transform.rotation.z = -1.f;
		grid.transform.position.x = -3.f;
		gameObjects.push_back(std::move(grid));
	}
}