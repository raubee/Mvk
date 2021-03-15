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

		BaseMaterial baseMaterial;

		void loadBaseMaterialShaders() const;

	public:
		void init(vk::Device device, RenderPass renderPass,
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
