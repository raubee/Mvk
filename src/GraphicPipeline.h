#pragma once
#include "VulkanVma.h"

namespace mvk
{
	class GraphicPipeline
	{
		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout descriptorSetLayout;

	public:
		
		GraphicPipeline(vk::Device device,
			vk::Extent2D extent,
			vk::RenderPass renderPass,
			std::vector<vk::PipelineShaderStageCreateInfo>
			shaderStageCreateInfos,
			vk::DescriptorSetLayout descriptorSetLayout);
		
		void release(vk::Device device);

		vk::DescriptorSetLayout getDescriptorSetLayout() const
		{
			return descriptorSetLayout;
		}

		vk::Pipeline getPipeline() const { return pipeline; }
		vk::PipelineLayout getPipelineLayout() const { return pipelineLayout; }
	};
}
