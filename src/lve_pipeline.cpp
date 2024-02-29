#include "lve_pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace lve {
	LvePipeline::LvePipeline(
			LveDevice &device,
			const std::string& vertFilePath,
			const std::string& fragFilePath,
			const PipelineConfigInfo& configInfo) 
			: lveDevice(device) {
		createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
	}

	LvePipeline::~LvePipeline() {
		vkDestroyShaderModule(lveDevice.device(), vertShaderModule, nullptr);
		vkDestroyShaderModule(lveDevice.device(), fragShaderModule, nullptr);
		vkDestroyPipeline(lveDevice.device(), graphicsPipeline, nullptr);
	}

	
	std::vector<char> LvePipeline::readFile(const std::string& filepath) {
		std::ifstream file{filepath, std::ios::ate | std::ios::binary};

		if (! file.is_open())
			throw std::runtime_error("Failed to open file: " + filepath); 
		
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}
	void LvePipeline::createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo) {

		assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configInfo");
		assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass  provided in configInfo");
		
		auto vertCode = readFile(vertFilePath);
		auto fragCode = readFile(fragFilePath);

		createShaderModule(vertCode, &vertShaderModule);
		createShaderModule(fragCode, &fragShaderModule);

		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;

		// See https://www.youtube.com/watch?v=ecMcXW6MSYU&t=268s
		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;				// 
		viewportInfo.viewportCount = 1;															// 
		viewportInfo.pViewports = &configInfo.viewport;											// 
		viewportInfo.scissorCount = 1;												 			// 
		viewportInfo.pScissors = &configInfo.scissor;											// 

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthSencilInfo;
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(lveDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("Failed to create graphics pipeline");


	}
	void LvePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(lveDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shader module");

	}
	PipelineConfigInfo LvePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
		PipelineConfigInfo configInfo{};

		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Rectangle formed by (0, 0, width, height) is the rectangle that will be drawn to by Vulkan. 
		// Setting to the size and position of the screen will have the effect of drawing directly to the screen.
		// Multiplying `configInfo.viewport.height` by 0.5 will draw everything within the top half of the display, effectively scrunching it
		configInfo.viewport.x = 0.0f;																		// Viewport Rectangle X coord
		configInfo.viewport.y = 0.0f;																		// Viewport Rectangle Y coord
		configInfo.viewport.width = static_cast<float>(width);												// Viewport Rectangle Width
		configInfo.viewport.height = static_cast<float>(height);											// Viewport Rectangle Height
		configInfo.viewport.minDepth = 0.0f;																// Viewport Min Depth
		configInfo.viewport.maxDepth = 1.0f;																// Viewport Max Depth
		// The min and max depth create a depth-range for the viewport

		// Rectangle formed by (offset.x, offset.y, extent.x, extent.y) is the rectangle that will act as a clipping rect for the viewport. 
		configInfo.scissor.offset = {0, 0};																	// Scissor Position Offset (position)
		configInfo.scissor.extent = {width, height};														// Scissor Position Extent (size)

		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;	// 
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;											// Disables Depth Clamping -- Allows Z components to be outside of [0, 1]
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;									// 
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;									// 
		configInfo.rasterizationInfo.lineWidth = 1.0f;														// 
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;											// Disable backface culling
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;									// Winding order of Triangles
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;											// 
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;	// Optional							// 
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;				// Optional							// 
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;		// Optional							// 

		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;
		configInfo.multisampleInfo.pSampleMask = nullptr;
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

		configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

		configInfo.depthSencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthSencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthSencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthSencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthSencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthSencilInfo.minDepthBounds = 0.0f;	// Optional
		configInfo.depthSencilInfo.maxDepthBounds = 1.0f;	// Optional
		configInfo.depthSencilInfo.stencilTestEnable = VK_FALSE	;
		configInfo.depthSencilInfo.front = {};	// Optional
		configInfo.depthSencilInfo.back = {};	// Optional

		return configInfo;
	}
	void LvePipeline::bind(VkCommandBuffer commandBuffer) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}
}