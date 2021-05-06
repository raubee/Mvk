#pragma once

#include "Device.hpp"

namespace mvk
{
	class GraphicPipeline
	{
		Device* ptrDevice;

		vk::Pipeline pipeline;
		vk::PipelineLayout pipelineLayout;
		vk::DescriptorSetLayout descriptorSetLayout;

	public:
		
		void build(Device* device,
			vk::Extent2D extent,
			vk::RenderPass renderPass,
			std::vector<vk::PipelineShaderStageCreateInfo>
			shaderStageCreateInfos,
			vk::DescriptorSetLayout* descriptorSetLayouts,
			uint32_t descriptorLayoutsSize,
			vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise);
		
		void release() const;

		vk::DescriptorSetLayout getDescriptorSetLayout() const
		{
			return descriptorSetLayout;
		}

		vk::Pipeline getPipeline() const { return pipeline; }
		vk::PipelineLayout getPipelineLayout() const { return pipelineLayout; }
	};
}
