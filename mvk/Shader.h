#pragma once

#include "Device.hpp"

namespace mvk
{
	class Shader
	{
		Device* device;

		std::string filename;
		vk::ShaderModule shaderModule;
		vk::ShaderStageFlagBits stageFlagBits;
		vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfo;

	public:
		Shader(Device* device, const std::string& filename,
		       vk::ShaderStageFlagBits stageFlagBits);

		void release() const;

		vk::PipelineShaderStageCreateInfo getPipelineShaderCreateInfo() const
		{
			return pipelineShaderStageCreateInfo;
		}
	};
}
