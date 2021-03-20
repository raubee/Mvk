#pragma once
#include "Shader.h"

namespace mvk
{
	class Material
	{
	protected:
		Shader* vertShader;
		Shader* fragShader;
		Shader* geoShader;
		Shader* tesShader;

		vma::Allocator allocator;
		vk::Device device;
		vk::CommandPool commandPool;
		vk::Queue transferQueue;

	public:
		Material();
		Material(Shader* vertShader, Shader* fragShader,
		         Shader* geoShader = nullptr, Shader* tesShader = nullptr);

		virtual void load(vma::Allocator allocator,
		                  vk::Device device,
		                  vk::CommandPool commandPool,
		                  vk::Queue transferQueue);

		virtual void release();

		virtual void writeDescriptorSet(vk::DescriptorSet descriptorSet)
		{
		};

		std::vector<vk::PipelineShaderStageCreateInfo>
		getPipelineShaderStageCreateInfo() const;
	};
}
