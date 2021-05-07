#include "GraphicPipeline.h"
#include "Vertex.h"

using namespace mvk;

void GraphicPipeline::build(Device* device,
                            GraphicPipelineCreateInfo createInfo)
{
	this->ptrDevice = device;

	/** Vertex Input State settings **/
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	const vk::PipelineVertexInputStateCreateInfo
		pipelineVertexInputStateCreateInfo = {
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &bindingDescription,
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(
				attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};

	/** Input Assembly **/
	const vk::PipelineInputAssemblyStateCreateInfo
		pipelineInputAssemblyStateCreateInfo = {
			.topology = vk::PrimitiveTopology::eTriangleList,
			.primitiveRestartEnable = false
		};

	/** Viewport and Scissors **/
	const vk::Viewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(createInfo.extent.width),
		.height = static_cast<float>(createInfo.extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	const vk::Rect2D scissor = {
		.offset = {
			.x = 0,
			.y = 0
		},
		.extent = createInfo.extent
	};

	const vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo =
	{
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	/** Rasterizer **/
	const vk::PipelineRasterizationStateCreateInfo
		pipelineRasterizationStateCreateInfo = {
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eBack,
			.frontFace = createInfo.frontFace,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f,
		};

	/** Multi-Sampling **/
	vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo =
	{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = VK_FALSE,
	};

	/** Depth and stencil **/
	vk::PipelineDepthStencilStateCreateInfo
		pipelineDepthStencilStateCreateInfo = {
			.depthTestEnable = vk::Bool32(true),
			.depthWriteEnable = vk::Bool32(true),
			.depthCompareOp = vk::CompareOp::eLess,
			.depthBoundsTestEnable = vk::Bool32(false),
			.stencilTestEnable = vk::Bool32(false),
		};

	/** Color blending **/
	vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {
		.blendEnable = vk::Bool32(createInfo.alpha),
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

	vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {
		.logicOpEnable = vk::Bool32(false),
		.attachmentCount = 1,
		.pAttachments = &pipelineColorBlendAttachmentState,
	};

	/** Dynamic state **/
	std::vector<vk::DynamicState> states = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eLineWidth,
		vk::DynamicState::eScissor,
	};

	vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {
		.dynamicStateCount = static_cast<uint32_t>(states.size()),
		.pDynamicStates = states.data()
	};

	/** Pipeline layout **/
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.setLayoutCount =
		static_cast<uint32_t>(createInfo.descriptorSetLayouts.size()),
		.pSetLayouts = createInfo.descriptorSetLayouts.data()
	};

	pipelineLayout =
		device->logicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);

	const vk::PipelineCache pipelineCache;
	const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
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
		.subpass = 0
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
	ptrDevice->logicalDevice.destroy(descriptorSetLayout);
	ptrDevice->logicalDevice.destroyPipelineLayout(pipelineLayout);
	ptrDevice->logicalDevice.destroyPipeline(pipeline);
}
