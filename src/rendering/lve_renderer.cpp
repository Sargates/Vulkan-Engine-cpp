#include "rendering/lve_renderer.hpp"

#include <iostream>
#include <stdexcept>
#include <array>
#include <vector>
#include <cmath>

namespace lve {

	LveRenderer::LveRenderer(LveWindow& window, LveDevice& device) : lveWindow{window}, lveDevice{device} {
		recreateSwapChain(); // Create swapchain for the first time
		createCommandBuffers();
	}
	LveRenderer::~LveRenderer() { freeCommandBuffers(); }


	void LveRenderer::recreateSwapChain() {
		auto extent = lveWindow.getExtent();

		while (extent.width == 0 || extent.width == 0) {
			extent = lveWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(lveDevice.device());
		if (lveSwapChain == nullptr) {
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
		} else {
			std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);

			if (! oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
				throw std::runtime_error("Swap chain image (or depth) format has changed");
			}
		}
		// We'll come back to this in just a moment
	}
	void LveRenderer::createCommandBuffers() {
		commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers");
	}
	void LveRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			lveDevice.device(),
			lveDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}
	VkCommandBuffer LveRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");
		
		auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) // Can fail if resizing window
			throw std::runtime_error("Failed to acquire swap chain image");
		
		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer");
		return commandBuffer;
	}
	void LveRenderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer");
		
		
		auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
			lveWindow.resetWindowResizedFlag();
			recreateSwapChain();
		} else if (result != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain iamge");
		
		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
	}
	void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0U};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Rectangle formed by (0, 0, width, height) is the rectangle that will be drawn to by Vulkan. 
		// Setting to the size and position of the screen will have the effect of drawing directly to the screen.
		// Multiplying `configInfo.viewport.height` by 0.5 will draw everything within the top half of the display, effectively scrunching it
		VkViewport viewport{};
		viewport.x = 0.0f;																	// Viewport Rectangle X coord
		viewport.y = static_cast<float>(lveSwapChain->getSwapChainExtent().height);			// Viewport Rectangle Y coord
		viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);		// Viewport Rectangle Width
		viewport.height = -static_cast<float>(lveSwapChain->getSwapChainExtent().height);	// Viewport Rectangle Height
		// The min and max depth create a depth-range for the viewport
		viewport.minDepth = 0.0f;															// Viewport Min Depth
		viewport.maxDepth = 1.0f;															// Viewport Max Depth
		// Rectangle formed by (offset.x, offset.y, extent.x, extent.y) is the rectangle that will act as a clipping rect for the viewport. 
		VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't finish render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);

	}

}