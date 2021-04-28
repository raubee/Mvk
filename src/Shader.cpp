#include "Shader.h"
#include <fstream>

using namespace mvk;

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error(
			std::string{"Failed to read file: "} + filename + "!");
	}

	const auto fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

Shader::Shader(const vk::Device device, const std::string& filename,
               const vk::ShaderStageFlagBits stageFlagBits)
{
	this->filename = filename;
	this->stageFlagBits = stageFlagBits;

	auto shaderCode = readFile(filename);

	const vk::ShaderModuleCreateInfo shaderModuleCreateInfo = {
		.codeSize = shaderCode.size(),
		.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data()),
	};

	shaderModule = device.createShaderModule(shaderModuleCreateInfo);

	pipelineShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo{
		.stage = stageFlagBits,
		.module = shaderModule,
		.pName = "main"
	};
}

void Shader::release(const vk::Device device) const
{
	if (shaderModule)
		device.destroyShaderModule(shaderModule);
}
