#include "simple_render_system.hpp"

#include <array>
#include <vector>
#include <cmath>

namespace lve {


	struct SimplePushConstantData {
		glm::mat4 transform{1.f};
		glm::mat4 normalMatrix{1.f};
	};

	SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass) : lveDevice{device} {
		createPipelineLayout();
		createPipeline(renderPass);
	}
	SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

	void SimpleRenderSystem::createPipelineLayout() {
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
	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice, 
			"obj/simple_shader.vert.spv", 
			"obj/simple_shader.frag.spv", 
			pipelineConfig
		);
	}
	void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects, const LveCamera& camera) {

		glm::mat4 projectionView = camera.getProjection() * camera.getView();
		
		lvePipeline->bind(commandBuffer);
		for (auto& obj : gameObjects) {

			
			
			SimplePushConstantData push;
			auto modelMatrix = obj.transform.getLocalToWorldMatrix(); // might be wrong
			push.transform = projectionView * modelMatrix;
			push.normalMatrix = obj.transform.getNormalMatrix();

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}
}