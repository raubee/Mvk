#include "Material.h"

using namespace mvk;

Material::Material()
{
}

Material::Material(Shader* vertShader, Shader* fragShader, Shader* geoShader,
                   Shader* tesShader)
{
	this->vertShader = vertShader;
	this->fragShader = fragShader;
	this->geoShader = geoShader;
	this->tesShader = tesShader;
}

void Material::load(const vma::Allocator allocator,
                    const vk::Device device,
                    const vk::CommandPool commandPool,
                    const vk::Queue transferQueue)
{
	this->allocator = allocator;
	this->device = device;
	this->commandPool = commandPool;
	this->transferQueue = transferQueue;
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
