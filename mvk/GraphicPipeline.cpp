#include "GraphicPipeline.h"
#include "Vertex.h"

using namespace mvk;

void GraphicPipeline::build(Device* device,
                            GraphicPipelineCreateInfo createInfo)
{
	this->ptrDevice = device;

	/** Vertex Input State settings **/
	const vk::PipelineVertexInputStateCreateInfo
		pipelineVertexInputStateCreateInfo{
			.vertexBindingDescriptionCount =
			static_cast<uint32_t>(createInfo
			                      .vertexInputBindingDescription.size()),

			.pVertexBindingDescriptions =
			createInfo.vertexInputBindingDescription.data(),

			.vertexAttributeDescriptionCount =
			static_cast<uint32_t>(createInfo
			                      .vertexInputAttributeDescription.size()),

			.pVertexAttributeDescriptions =
			createInfo.vertexInputAttributeDescription.data()
		};

	/** Input Assembly **/
	const vk::PipelineInputAssemblyStateCreateInfo
		pipelineInputAssemblyStateCreateInfo{
			.topology = vk::PrimitiveTopology::eTriangleList,
			.primitiveRestartEnable = false
		};

	/** Viewport and Scissors **/
	// Note: Viewport and Scissors will be updated dynamically with command buffers
	const vk::Viewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = 0.0f,
		.height = 0.0f,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	const vk::Rect2D scissor{
		.offset{
			.x = 0,
			.y = 0
		},
		.extent{0, 0}
	};

	const vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	/** Rasterizer **/
	const vk::PipelineRasterizationStateCreateInfo
		pipelineRasterizationStateCreateInfo{
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = createInfo.cullMode,
			.frontFace = createInfo.frontFace,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f,
		};

	/** Multi-Sampling **/
	const vk::PipelineMultisampleStateCreateInfo
		pipelineMultisampleStateCreateInfo{
			.rasterizationSamples = ptrDevice->multiSampling,
			.sampleShadingEnable = VK_FALSE,
		};

	/** Depth and stencil **/
	const vk::PipelineDepthStencilStateCreateInfo
		pipelineDepthStencilStateCreateInfo{
			.depthTestEnable = createInfo.depthTest,
			.depthWriteEnable = vk::Bool32(true),
			.depthCompareOp = vk::CompareOp::eLess,
			.depthBoundsTestEnable = vk::Bool32(false),
			.stencilTestEnable = vk::Bool32(false),
		};

	/** Color blending **/
	const vk::PipelineColorBlendAttachmentState
		pipelineColorBlendAttachmentState{
			.blendEnable = createInfo.alpha,
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.alphaBlendOp = vk::BlendOp::eAdd,
			.colorWriteMask =
			vk::ColorComponentFlagBits::eR |
			vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB |
			vk::ColorComponentFlagBits::eA,
		};

	const vk::PipelineColorBlendStateCreateInfo
		pipelineColorBlendStateCreateInfo{
			.logicOpEnable = vk::Bool32(false),
			.attachmentCount = 1,
			.pAttachments = &pipelineColorBlendAttachmentState,
		};

	/** Dynamic state **/
	const std::vector<vk::DynamicState> states{
		vk::DynamicState::eViewport,
		vk::DynamicState::eLineWidth,
		vk::DynamicState::eScissor
	};

	const vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{
		.dynamicStateCount = static_cast<uint32_t>(states.size()),
		.pDynamicStates = states.data()
	};

	/** Pipeline layout **/
	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.setLayoutCount =
		static_cast<uint32_t>(createInfo.descriptorSetLayouts.size()),
		.pSetLayouts = createInfo.descriptorSetLayouts.data(),
		.pushConstantRangeCount =
		static_cast<uint32_t>(createInfo.pushConstantRanges.size()),
		.pPushConstantRanges = createInfo.pushConstantRanges.data()
	};

	pipelineLayout =
		device->logicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);

	const vk::PipelineCache pipelineCache;
	const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
		.stageCount =
		static_cast<uint32_t>(createInfo.shaderStageCreateInfos.size()),
		.pStages = createInfo.shaderStageCreateInfos.data(),
		.pVertexInputState = &pipelineVertexInputStateCreateInfo,
		.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
		.pViewportState = &pipelineViewportStateCreateInfo,
		.pRasterizationState = &pipelineRasterizationStateCreateInfo,
		.pMultisampleState = &pipelineMultisampleStateCreateInfo,
		.pDepthStencilState = &pipelineDepthStencilStateCreateInfo,
		.pColorBlendState = &pipelineColorBlendStateCreateInfo,
		.pDynamicState = &pipelineDynamicStateCreateInfo,
		.layout = pipelineLayout,
		.renderPass = createInfo.renderPass,
		.subpass = 0,
	};

	vk::Result result;
	std::tie(result, pipeline) =
		device->logicalDevice.createGraphicsPipeline(pipelineCache,
		                                             graphicsPipelineCreateInfo);

	switch (result)
	{
	case vk::Result::eSuccess: break;
	case vk::Result::ePipelineCompileRequiredEXT:
		// something meaningfull here
		break;
	default: assert(false); // should never happen
	}
}

void GraphicPipeline::release() const
{
	ptrDevice->logicalDevice.destroyPipelineLayout(pipelineLayout);
	ptrDevice->logicalDevice.destroyPipeline(pipeline);
}
