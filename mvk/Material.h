#pragma once

#include "Shader.h"

#define VMA_HPP_NO_STRUCT_CONSTRUCTORS
#include "../3rdParty/vk_mem_alloc.hpp"

namespace mvk
{
	class Material
	{
	protected:

		Shader* vertShader;
		Shader* fragShader;
		Shader* geoShader;
		Shader* tesShader;

		vk::Extent2D extent;
		vk::CommandPool commandPool;
		vk::DescriptorPool descriptorPool;
		vk::DescriptorSetLayout descriptorSetLayout;
		std::vector<vk::DescriptorSet> descriptorSets;

	public:
		Material(vk::Device device, Shader* vertShader, Shader* fragShader,
		         Shader* geoShader = nullptr, Shader* tesShader = nullptr);

		virtual void release(vk::Device device);

		std::vector<vk::PipelineShaderStageCreateInfo>
		getPipelineShaderStageCreateInfo() const;

		vk::DescriptorSetLayout getDescriptorSetLayout() const
		{
			return descriptorSetLayout;
		}

		vk::DescriptorSet getDescriptorSet(const int index) const
		{
			if (descriptorSets.size() <= index) return nullptr;

			return descriptorSets[index];
		}
	};
}
