#pragma once

#include "RenderPass.h"
#include "BaseMaterial.h"

namespace mvk
{
	class GraphicPipeline
	{
		vk::Device device;
		vk::Pipeline pipeline;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::PipelineLayout pipelineLayout;

	public:
		void init(vk::Device device,
		          std::vector<vk::PipelineShaderStageCreateInfo>
		          shaderStageCreateInfos,
		          RenderPass renderPass,
		          vk::Extent2D extent);
		void release();

		vk::Pipeline getPipeline() const { return pipeline; }
		vk::PipelineLayout getPipelineLayout() const { return pipelineLayout; }

		vk::DescriptorSetLayout getDescriptorSetLayout() const
		{
			return descriptorSetLayout;
		}
	};
}
