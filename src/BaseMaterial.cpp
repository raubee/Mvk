#include "BaseMaterial.h"

using namespace mvk;

BaseMaterial::BaseMaterial(const vk::Device device,
                           const vma::Allocator allocator,
                           const BaseMaterialDescription description) :
	Material(device,
	         allocator,
	         new Shader(device, "shaders/vert.spv",
	                    vk::ShaderStageFlagBits::eVertex),
	         new Shader(device, "shaders/frag.spv",
	                    vk::ShaderStageFlagBits::eFragment)),
	albedo(nullptr)
{
	this->description = description;
}

void BaseMaterial::init(const vk::Device device, vma::Allocator allocator,
                        const uint32_t size)
{
	createDescriptorPool(device, size);
}

void BaseMaterial::createDescriptorPool(const vk::Device device,
                                        const uint32_t size)
{
	vk::DescriptorPoolSize descriptorPoolSize{
		.type = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = size
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		.maxSets = size,
		.poolSizeCount = 1,
		.pPoolSizes = &descriptorPoolSize
	};

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);
}

vk::DescriptorSetLayout BaseMaterial::getDescriptorSetLayout(
	const vk::Device device)
{
	/** Descriptor Set layout **/
	vk::DescriptorSetLayoutBinding uniformBufferLayoutBinding = {
		.binding = 0,
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eVertex
	};

	vk::DescriptorSetLayoutBinding albedoLayoutBinding = {
		.binding = 1,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eFragment
	};

	std::array<vk::DescriptorSetLayoutBinding, 2> layoutBindings
		= {uniformBufferLayoutBinding, albedoLayoutBinding};

	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
		.pBindings = layoutBindings.data()
	};

	return device.createDescriptorSetLayout(
		descriptorSetLayoutCreateInfo);
}

void BaseMaterial::writeDescriptorSet(const vk::Device device,
                                      const vk::DescriptorSet descriptorSet)
{
	vk::DescriptorImageInfo albedoDescriptorImageInfo = {
		.sampler = albedo->getSampler(),
		.imageView = albedo->getImageView(),
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};

	vk::WriteDescriptorSet writeDescriptorSet = {
		.dstSet = descriptorSet,
		.dstBinding = 1,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.pImageInfo = &albedoDescriptorImageInfo
	};

	device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

void BaseMaterial::release(const vk::Device device)
{
	Material::release(device);
}
