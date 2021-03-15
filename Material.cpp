#include "Material.h"

using namespace mvk;

Material::Material(const std::string& vertShader,
                   const std::string& fragShader,
                   const std::string& geoShader,
                   const std::string& tesShader)
{
	this->vertShader = new Shader(vertShader,
	                              vk::ShaderStageFlagBits::eVertex);
	this->fragShader = new Shader(fragShader,
	                              vk::ShaderStageFlagBits::eFragment);
	if (!geoShader.empty())
	{
		this->geoShader = new Shader(geoShader,
		                             vk::ShaderStageFlagBits::eGeometry);
	}
	else this->geoShader = nullptr;

	if (!tesShader.empty())
	{
		this->tesShader = new Shader(tesShader,
		                             vk::ShaderStageFlagBits::
		                             eTessellationControl);
	}
	else this->tesShader = nullptr;
}

Material::Material(Shader vertShader, Shader fragShader, Shader geoShader,
                   Shader tesShader)
{
	this->vertShader = &vertShader;
	this->fragShader = &fragShader;
	this->geoShader = &geoShader;
	this->tesShader = &tesShader;
}

void Material::loadShaders(const vk::Device device) const
{
	vertShader->load(device);
	fragShader->load(device);

	if (geoShader != nullptr)
		geoShader->load(device);

	if (tesShader != nullptr)
		tesShader->load(device);
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
		pipelineShaderStageCreateInfos = {
			vertShader->getPipelineShaderCreateInfo(),
			fragShader->getPipelineShaderCreateInfo()
		};
	return pipelineShaderStageCreateInfos;
}
