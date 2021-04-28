#pragma once
#include "Shader.h"
#include "VulkanVma.h"
#include "GraphicPipeline.h"

namespace mvk
{
	class Material
	{
		GraphicPipeline* pipeline;


	protected:
		Shader* vertShader;
		Shader* fragShader;
		Shader* geoShader;
		Shader* tesShader;

		vk::Extent2D extent;
		vk::CommandPool commandPool;
		vk::DescriptorPool descriptorPool;

	public:
		Material(vk::Device device, vma::Allocator allocator,
		         Shader* vertShader, Shader* fragShader,
		         Shader* geoShader = nullptr, Shader* tesShader = nullptr);

		virtual void init(vk::Device device, vma::Allocator allocator,
		                  uint32_t size)
		{
		}

		virtual vk::DescriptorSetLayout getDescriptorSetLayout(
			vk::Device device);

		virtual void release(vk::Device device);

		std::vector<vk::PipelineShaderStageCreateInfo>
		getPipelineShaderStageCreateInfo() const;

		void setGraphicPipeline(GraphicPipeline* pipeline)
		{
			this->pipeline = pipeline;
		}

		GraphicPipeline* getGraphicPipeline() { return pipeline; }
	};
}
