#pragma once

#include "Vulkan.h"

namespace mvk
{
	class Shader
	{
		std::string filename;
		vk::Device device;
		vk::ShaderModule shaderModule;
		vk::ShaderStageFlagBits stageFlagBits;
		vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;

	public:
		Shader(const std::string& filename,
		       vk::ShaderStageFlagBits stageFlagBits);

		void load(vk::Device device);
		void release();

		vk::PipelineShaderStageCreateInfo getPipelineShaderCreateInfo() const
		{
			return pipelineShaderStageCreateInfo;
		}
	};
}
