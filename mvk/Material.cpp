#include "Material.h"

using namespace mvk;

void Material::load(Shader* vertShader,
                    Shader* fragShader,
                    Shader* geoShader,
                    Shader* tesShader)
{
	this->vertShader = vertShader;
	this->fragShader = fragShader;
	this->geoShader = geoShader;
	this->tesShader = tesShader;
}

void Material::release(const Device device)
{
	if (vertShader != nullptr)
		vertShader->release(device);

	if (fragShader != nullptr)
		fragShader->release(device);

	if (geoShader != nullptr)
		geoShader->release(device);

	if (tesShader != nullptr)
		tesShader->release(device);
}

std::vector<vk::PipelineShaderStageCreateInfo> Material::
getPipelineShaderStageCreateInfo() const
{
	std::vector<vk::PipelineShaderStageCreateInfo>
		pipelineShaderStageCreateInfos;

	if (vertShader != nullptr)
		pipelineShaderStageCreateInfos
			.push_back(vertShader->getPipelineShaderCreateInfo());

	if (fragShader != nullptr)
		pipelineShaderStageCreateInfos
			.push_back(fragShader->getPipelineShaderCreateInfo());

	if (geoShader != nullptr)
		pipelineShaderStageCreateInfos
			.push_back(geoShader->getPipelineShaderCreateInfo());

	if (tesShader != nullptr)
		pipelineShaderStageCreateInfos
			.push_back(tesShader->getPipelineShaderCreateInfo());

	return pipelineShaderStageCreateInfos;
}
