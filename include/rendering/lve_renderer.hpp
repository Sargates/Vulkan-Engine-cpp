#pragma once

#include "rendering/lve_window.hpp"
#include "rendering/lve_device.hpp"
#include "rendering/lve_swap_chain.hpp"
#include "rendering/lve_model.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace lve {
	class LveRenderer {
		public:
			LveRenderer(LveWindow& window, LveDevice& device);
			~LveRenderer();

			LveRenderer(const LveRenderer &) = delete;
			LveRenderer& operator=(const LveRenderer &) = delete;

			VkRenderPass getSwapChainRenderPass() const { return lveSwapChain-> getRenderPass(); }
			float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
			bool isFrameInProgress() const { return isFrameStarted; }

			VkCommandBuffer getCurrentCommandBuffer() const {
				assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
				return commandBuffers[currentFrameIndex];
			}

			int getFrameIndex() {
				assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
				return currentFrameIndex;
			}

			VkCommandBuffer beginFrame();
			void endFrame();
			void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapChainRenderPass(VkCommandBuffer commandBuffer);


		private:
			void createCommandBuffers();
			void freeCommandBuffers();
			void recreateSwapChain();

			uint32_t currentImageIndex;
			int currentFrameIndex = 0; // on [0, MAX_FRAMES_IN_FLIGHT) -- `MAX_FRAMES_IN_FLIGHT` is defined in `lve_swap_chain.hpp`
			bool isFrameStarted = false;

			LveWindow& lveWindow;
			LveDevice& lveDevice;
			std::unique_ptr<LveSwapChain> lveSwapChain;
			std::vector<VkCommandBuffer> commandBuffers;
	};
}