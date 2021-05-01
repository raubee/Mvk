#pragma once

#include "Device.hpp"

namespace mvk
{
	class Shader
	{
		std::string filename;
		vk::ShaderModule shaderModule;
		vk::ShaderStageFlagBits stageFlagBits;
		vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;

	public:
		Shader(Device device, const std::string& filename,
		       vk::ShaderStageFlagBits stageFlagBits);

		void release(Device device) const;

		vk::PipelineShaderStageCreateInfo getPipelineShaderCreateInfo() const
		{
			return pipelineShaderStageCreateInfo;
		}
	};
}
