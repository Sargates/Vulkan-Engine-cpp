#include "first_app.hpp"


#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>
#include <cmath>

namespace lve {

	//! Sierpinski's Triangle Generator
	void sierpinski(
			std::vector<LveModel::Vertex> &vertices,
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
	struct SimplePushConstantData {
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	FirstApp::FirstApp() {
		loadModels();
		createPipelineLayout();
		recreateSwapChain(); // Create swapchain for the first time
		createCommandBuffers();
	}
	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
		// vkDestroyPipeline();
	}

	void FirstApp::run() {
		std::cout << "maxPushConstantSize = " << lveDevice.properties.limits.maxPushConstantsSize << std::endl;
		std::cout << "sizeof(SimplePushConstantData) = " << sizeof(SimplePushConstantData) << std::endl;
		
		while (! lveWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
			if (glfwGetKey(lveWindow.getWindow(), GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(lveWindow.getWindow(), GLFW_TRUE); }
		}

		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadModels() {
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

		lveModel = std::make_unique<LveModel>(lveDevice, vertices);
	}

	void FirstApp::createPipelineLayout() {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);


		
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout");
	}

	void FirstApp::recreateSwapChain() {
		auto extent = lveWindow.getExtent();

		while (extent.width == 0 || extent.width == 0) {
			extent = lveWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(lveDevice.device());
		if (lveSwapChain == nullptr) {
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
		} else {
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
			if (lveSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
		createPipeline();
	}

	void FirstApp::createPipeline() {
		assert(  lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = lveSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice, 
			"obj/simple_shader.vert.spv", 
			"obj/simple_shader.frag.spv", 
			pipelineConfig
		);
	}

	void FirstApp::createCommandBuffers() {
		commandBuffers.resize(lveSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers");

		for (int i=0; i<commandBuffers.size(); i++)
			recordCommandBuffer(i);
	}

	void FirstApp::freeCommandBuffers() {
		vkFreeCommandBuffers(
			lveDevice.device(),
			lveDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}

	void FirstApp::recordCommandBuffer(int imageIndex) {
		static int frame = 0;
		frame = (frame+1)%1000;
		
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer. Index " + imageIndex);
		
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0U};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);
		
		
		lvePipeline->bind(commandBuffers[imageIndex]);
		lveModel->bind(commandBuffers[imageIndex]);

		for (int j=0; j<4; j++) {
			SimplePushConstantData push;
			push.offset = { -0.5f + frame*0.001f, -0.5f + 0.25f*j };
			push.color = { 0.0f, 0.0f, 0.0f + 0.25f*j };

			vkCmdPushConstants(
				commandBuffers[imageIndex],
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);
			lveModel->draw(commandBuffers[imageIndex]);
		}
		

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer. Index " + imageIndex);
	}

	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = lveSwapChain->acquireNextImage(&imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) // Can fail if resizing window
			throw std::runtime_error("Failed to acquire swap chain image");
			
		recordCommandBuffer(imageIndex);
		result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
			lveWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		} 
		if (result != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain iamge");
	}

}