#pragma once

#include "Vulkan.h"

namespace mvk
{
	class Shader
	{
		vk::Device device;
		std::string filename;
		vk::ShaderModule shaderModule;
		vk::ShaderStageFlagBits stageFlagBits;
		vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;

	public:
		Shader(vk::Device device, const std::string& filename,
		       vk::ShaderStageFlagBits stageFlagBits);

		void release() const;

		vk::PipelineShaderStageCreateInfo getPipelineShaderCreateInfo() const
		{
			return pipelineShaderStageCreateInfo;
		}
	};
}
