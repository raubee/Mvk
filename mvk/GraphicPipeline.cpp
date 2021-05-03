#include "GraphicPipeline.h"
#include "Vertex.h"

using namespace mvk;

void GraphicPipeline::build(vk::Device device,
                            vk::Extent2D extent,
                            vk::RenderPass renderPass,
                            std::vector<vk::PipelineShaderStageCreateInfo>
                            shaderStageCreateInfos,
                            vk::DescriptorSetLayout* descriptorSetLayouts,
                            uint32_t descriptorLayoutsSize,
							vk::FrontFace frontFace)
{
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
		.width = static_cast<float>(extent.width),
		.height = static_cast<float>(extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};

	const vk::Rect2D scissor = {
		.offset = {
			.x = 0,
			.y = 0
		},
		.extent = extent
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
			.frontFace = frontFace,
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
		.blendEnable = vk::Bool32(false),
		.colorWriteMask =
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA
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
		.setLayoutCount = descriptorLayoutsSize,
		.pSetLayouts = descriptorSetLayouts
	};

	pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	const vk::PipelineCache pipelineCache;
	const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
		.stageCount =
		static_cast<uint32_t>(shaderStageCreateInfos.size()),
		.pStages = shaderStageCreateInfos.data(),
		.pVertexInputState = &pipelineVertexInputStateCreateInfo,
		.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
		.pViewportState = &pipelineViewportStateCreateInfo,
		.pRasterizationState = &pipelineRasterizationStateCreateInfo,
		.pMultisampleState = &pipelineMultisampleStateCreateInfo,
		.pDepthStencilState = &pipelineDepthStencilStateCreateInfo,
		.pColorBlendState = &pipelineColorBlendStateCreateInfo,
		.pDynamicState = &pipelineDynamicStateCreateInfo,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = 0
	};

	vk::Result result;
	std::tie(result, pipeline) = device.createGraphicsPipeline(
		pipelineCache, graphicsPipelineCreateInfo);

	switch (result)
	{
	case vk::Result::eSuccess: break;
	case vk::Result::ePipelineCompileRequiredEXT:
		// something meaningfull here
		break;
	default: assert(false); // should never happen
	}
}

void GraphicPipeline::release(const vk::Device device)
{
	if (descriptorSetLayout)
		device.destroy(descriptorSetLayout);

	if (pipelineLayout)
		device.destroyPipelineLayout(pipelineLayout);

	if (pipeline)
		device.destroyPipeline(pipeline);
}
