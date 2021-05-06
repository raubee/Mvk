#include "Material.h"

using namespace mvk;

void Material::load(Device* device,
                    Shader* vertShader,
                    Shader* fragShader,
                    Shader* geoShader,
                    Shader* tesShader)
{
	this->ptrDevice = device;

	this->vertShader = vertShader;
	this->fragShader = fragShader;
	this->geoShader = geoShader;
	this->tesShader = tesShader;
}

void Material::release()
{
	if (vertShader != nullptr)
		vertShader->release();

	if (fragShader != nullptr)
		fragShader->release();

	if (geoShader != nullptr)
		geoShader->release();

	if (tesShader != nullptr)
		tesShader->release();
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
