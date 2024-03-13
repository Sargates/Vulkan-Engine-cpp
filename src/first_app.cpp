#include "first_app.hpp"

#include "rendering/lve_camera.hpp"
#include "rendering/systems/simple_render_system.hpp"
#include "rendering/systems/point_light_system.hpp"
#include "keyboard_movement_controller.hpp"
#include "rendering/lve_buffer.hpp"
#include "math.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <chrono>
#include <vector>

#define print(x) std::cout << x << std::endl

glm::vec2 lastMousePos{-1.0};


namespace lve {

	LveCamera* activeCamera = nullptr;


	FirstApp::FirstApp() {
		globalPool = LveDescriptorPool::Builder{lveDevice}
			.setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		loadGameObjects();
	}
	FirstApp::~FirstApp() {}

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
		glm::vec2 delta = (currentPos-lastMousePos) * 0.001f;
		
		activeCamera->transform.rotation.x += delta.y; // Delta-Y maps to rotation about X-axis
		activeCamera->transform.rotation.y += delta.x; // Delta-X maps to rotation about Y-axis
		float max = glm::half_pi<float>() - 0.01f; // A little smaller than pi/2
		float min = 0.01f - glm::half_pi<float>(); // A little bigger than -pi/2
		activeCamera->transform.rotation.x = std::clamp(activeCamera->transform.rotation.x, min, max);

		activeCamera->UpdateViewMatrix();
		lastMousePos = currentPos;
	}
	void checkKeys(GLFWwindow* window, float dt) {
		glm::vec3 keyVector{};
		if (glfwGetKey(window, GLFW_KEY_W))          { keyVector.z += 1; }
		if (glfwGetKey(window, GLFW_KEY_S))          { keyVector.z -= 1; }
		if (glfwGetKey(window, GLFW_KEY_D))          { keyVector.x += 1; }
		if (glfwGetKey(window, GLFW_KEY_A))          { keyVector.x -= 1; }
		if (glfwGetKey(window, GLFW_KEY_SPACE))      { keyVector.y += 1; }
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) { keyVector.y -= 1; }

		glm::vec3& forward=activeCamera->transform.forward;
		glm::vec3&   right=activeCamera->transform.right;
		glm::vec2 forwardXZ = glm::normalize(glm::vec2{forward.x, forward.z}) * keyVector.z;
		glm::vec2   rightXZ = glm::normalize(glm::vec2{right.x, right.z})     * keyVector.x;
		glm::vec2   finalXZ = (keyVector.z != 0 || keyVector.x != 0) ? glm::normalize(forwardXZ + rightXZ) : glm::zero<glm::vec3>();
		activeCamera->transform.position += dt * glm::vec3{finalXZ.x, keyVector.y, finalXZ.y}; // finalXZ is a 2d vector - use finalXZ.y; no Z comp
		activeCamera->UpdateViewMatrix();
	}


	void FirstApp::run() {

		std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i=0; i<uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<LveBuffer>(
				lveDevice,
				sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = LveDescriptorSetLayout::Builder{lveDevice}
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		
		std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i=0; i<globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			LveDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		//! This fucking solution of needing to have a pointer you need to cast and dereference is the stupidest thing ever. Fair enough because C++ doesn't have (adequate) reflection.
		// glfwSetWindowUserPointer(lveWindow.getWindow(), this);

		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
		PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

		// Cameras
		LveCamera camera{};
		activeCamera = &camera; // Set `activeCamera` to this local camera
		KeyboardMovementController cameraController{};
		auto viewerObject = LveGameObject::createGameObject();
		camera.transform.position = {0.f, 1.f, -2.5f};
		camera.transform.rotation = glm::zero<glm::vec3>();

		// glfwSetKeyCallback(lveWindow.getWindow(), handleKey);
		glfwSetInputMode(lveWindow.window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(lveWindow.window(), mouseCallback);


		// Clock
		auto currentTime = std::chrono::high_resolution_clock::now();


		float totalTime = 0.f;
		while (! lveWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			totalTime += dt;
			currentTime = newTime;
			int FPS = (dt <= 0.001f) ? 999 : 1.0f/dt;
			std::cout << FPS << "\r";


			checkKeys(lveWindow.window(), dt);

			float aspectRatio = lveRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(90.f), aspectRatio, 0.01f, 100.f);

			if (auto commandBuffer = lveRenderer.beginFrame()) {
				int frameIndex = lveRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					dt,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects};

				// Update
				GlobalUBO ubo{};
				update(ubo, frameInfo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();


				// Render
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.renderLights(frameInfo);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
			if (glfwGetKey(lveWindow.window(), GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(lveWindow.window(), GLFW_TRUE); }
		}
		std::cout << std::endl;

		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::update(GlobalUBO &ubo, FrameInfo &frameInfo) {
		ubo.projectionMatrix = frameInfo.camera.getProjection();
		ubo.cameraMatrix = frameInfo.camera.transform.getWorldToLocal();
		ubo.invCameraMatrix = frameInfo.camera.transform.getLocalToWorld();

		auto rotateLights = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, {0.f, 1.f, 0.f});
		int lightIndex = 0;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			// update light position
			obj.transform.position = glm::vec3(rotateLights * glm::vec4(obj.transform.position, 1.f));

			// copy light to ubo
			ubo.lights[lightIndex].position = obj.transform.position;
			ubo.lights[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
			if (++lightIndex == MAX_LIGHTS) break;
		}
		ubo.numLights = lightIndex;
	}

	void FirstApp::loadGameObjects() {

		
		std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "resources/flat_vase.obj");
		auto flatVase = LveGameObject::createGameObject();
		flatVase.model = lveModel;
		flatVase.transform.position = {-.5f, -.5f, 0.f};
		flatVase.transform.scale = {3.f, 1.5f, 3.f};
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));

		lveModel = LveModel::createModelFromFile(lveDevice, "resources/smooth_vase.obj");
		auto smoothVase = LveGameObject::createGameObject();
		smoothVase.model = lveModel;
		smoothVase.transform.position = {.5f, -.5f, 0.f};
		smoothVase.transform.scale = {3.f, 1.5f, 3.f};
		gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		lveModel = createGrid(lveDevice, glm::vec3{0.f}, 0.5, glm::ivec2{16});
		auto floor = LveGameObject::createGameObject();
		floor.model = lveModel;
		floor.transform.position = {0.f, -.5f, .5f};
		floor.transform.scale = {3.f, 1.f, 3.f};
		gameObjects.emplace(floor.getId(), std::move(floor));

		// https://www.color-hex.com/color-palette/5361
		std::vector<glm::vec3> colors{
			{1.f, .1f, .1f},
			{1.f, .647f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, .1f},
			{.1f, .1f, 1.f},
			{1.f, .1f, 1.f},
		};

		for (int i = 0; i < std::min(colors.size(), (size_t)MAX_LIGHTS); i++) {
			auto pointLight = LveGameObject::makePointLight(0.2f);
			pointLight.color = colors[i];
			auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / colors.size(), {0.f, -1.f, 0.f});
			pointLight.transform.position = glm::vec3(rotateLight * glm::vec4{1.f});
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}

		// lveModel = LveModel::createModelFromFile(lveDevice, "./resources/icosphere.obj");
		// LveGameObject icosphere = LveGameObject::createGameObject();
		// icosphere.model = lveModel;
		// icosphere.transform.scale = glm::vec3{0.1f};
		// gameObjects.emplace(icosphere.getId(), std::move(icosphere));


		// // XYZ Basis Grid (minecraft-like)
		// LveModel::Builder builder{};
		// builder.loadModel("./resources/cube.obj");
		
		// LveGameObject xVector = LveGameObject::createGameObject();
		// for (auto& vertex : builder.vertices) { vertex.color = {1.f, 0.f, 0.f}; } // Red
		// xVector.model = std::make_unique<LveModel>(lveDevice, builder);
		// xVector.transform.position = {0.f, 1.f, 0.f};
		// xVector.transform.scale = {.5f, 0.05f, 0.05f};
		// gameObjects.emplace(xVector.getId(), std::move(xVector));

		// LveGameObject yVector = LveGameObject::createGameObject();
		// for (auto& vertex : builder.vertices) { vertex.color = {0.f, 1.f, 0.f}; } // Green
		// yVector.model = std::make_unique<LveModel>(lveDevice, builder);
		// yVector.transform.position = {0.f, 1.f, 0.f};
		// yVector.transform.scale = {0.05f, .5f, 0.05f};
		// gameObjects.emplace(yVector.getId(), std::move(yVector));
		
		// LveGameObject zVector = LveGameObject::createGameObject();
		// for (auto& vertex : builder.vertices) { vertex.color = {0.f, 0.f, 1.f}; } // Blue
		// zVector.model = std::make_unique<LveModel>(lveDevice, builder);
		// zVector.transform.position = {0.f, 1.f, 0.f};
		// zVector.transform.scale = {0.05f, 0.05f, .5f};
		// gameObjects.emplace(zVector.getId(), std::move(zVector));

	}
}