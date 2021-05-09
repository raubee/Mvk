#pragma once

#include "Device.hpp"

namespace mvk
{
	struct GraphicPipelineCreateInfo
	{
		std::vector<vk::VertexInputBindingDescription>
		vertexInputBindingDescription;
		std::vector<vk::VertexInputAttributeDescription>
		vertexInputAttributeDescription;
		vk::RenderPass renderPass;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		vk::FrontFace frontFace;
		vk::Bool32 alpha = vk::Bool32(false);
		vk::Bool32 depthTest = vk::Bool32(true);
	};

	class GraphicPipeline
	{
		Device* ptrDevice;

		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;

	public:

		void build(Device* device,
		           GraphicPipelineCreateInfo createInfo);

		void release() const;

		vk::Pipeline getPipeline() const { return pipeline; }
		vk::PipelineLayout getPipelineLayout() const { return pipelineLayout; }
	};
}
