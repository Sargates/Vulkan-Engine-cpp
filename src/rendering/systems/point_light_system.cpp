#include "rendering/systems/point_light_system.hpp"

// libs
#include "math.hpp"

// std
#include <array>
#include <cassert>
#include <stdexcept>
#include <map>

namespace lve {

	struct PointLightPushConstants {
		glm::vec3 position{};
		alignas(16) glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lveDevice{device} {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}
	PointLightSystem::~PointLightSystem() { vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr); }

	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo configInfo{};
		LvePipeline::defaultPipelineConfigInfo(configInfo);
		LvePipeline::enableAlphaBlending(configInfo);
		configInfo.renderPass = renderPass;
		configInfo.pipelineLayout = pipelineLayout;
		configInfo.bindingDescriptions.clear();
		configInfo.attributeDescriptions.clear();

		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"obj/point_light.vert.spv",
			"obj/point_light.frag.spv",
			configInfo);
	}

	void PointLightSystem::renderLights(FrameInfo& frameInfo) {

		// Sort lights
		std::map<float, LveGameObject::id_t> sorted;
		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.pointLight == nullptr) continue;

			glm::vec3 displacement = frameInfo.camera.transform.position - obj.transform.position;
			float disSquared = glm::dot(displacement, displacement);
			sorted[disSquared] = obj.getId();
		}

		

		lvePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr);

		// for (auto& kv : frameInfo.gameObjects) {
		// 	auto& obj = kv.second;
		// 	if (obj.pointLight == nullptr) continue;
		for (auto it = sorted.rbegin(); it != sorted.rend(); it++) {
			auto& obj = frameInfo.gameObjects.at(it->second);

			PointLightPushConstants push{};
			push.position = obj.transform.position;
			push.color = obj.pointLight->color;
			push.radius = .04f;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}

}	// namespace lve
