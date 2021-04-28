#pragma once

#include "Vulkan.h"

namespace mvk
{
	class Shader
	{
		std::string filename;
		vk::ShaderModule shaderModule;
		vk::ShaderStageFlagBits stageFlagBits;
		vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;

	public:
		Shader(vk::Device device, const std::string& filename,
		       vk::ShaderStageFlagBits stageFlagBits);

		void release(vk::Device device) const;

		vk::PipelineShaderStageCreateInfo getPipelineShaderCreateInfo() const
		{
			return pipelineShaderStageCreateInfo;
		}
	};
}
