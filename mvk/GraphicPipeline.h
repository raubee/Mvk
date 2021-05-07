#pragma once

#include "Device.hpp"

namespace mvk
{
	struct GraphicPipelineCreateInfo
	{
		vk::Extent2D extent;
		vk::RenderPass renderPass;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		vk::FrontFace frontFace;
		bool alpha;
	};

	class GraphicPipeline
	{
		Device* ptrDevice;

		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout descriptorSetLayout;

	public:

		void build(Device* device,
		           GraphicPipelineCreateInfo createInfo);

		void release() const;

		vk::DescriptorSetLayout getDescriptorSetLayout() const
		{
			return descriptorSetLayout;
		}

		vk::Pipeline getPipeline() const { return pipeline; }
		vk::PipelineLayout getPipelineLayout() const { return pipelineLayout; }
	};
}
