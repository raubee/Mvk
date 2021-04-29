#include "BaseMaterial.h"

using namespace mvk;

BaseMaterial::BaseMaterial(const vk::Device device,
                           const BaseMaterialDescription description) :
	Material(new Shader(device, "shaders/vert.spv",
	                    vk::ShaderStageFlagBits::eVertex),
	         new Shader(device, "shaders/frag.spv",
	                    vk::ShaderStageFlagBits::eFragment)),
	albedo(description.albedo)
{
}

void BaseMaterial::init(const vk::Device device, const vma::Allocator allocator)
{
	Material::init(device, allocator);
}

void BaseMaterial::createDescriptorPool(const vk::Device device)
{
	vk::DescriptorPoolSize descriptorPoolSize{
		.type = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = 1
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		.maxSets = 1,
		.poolSizeCount = 1,
		.pPoolSizes = &descriptorPoolSize
	};

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);
}

void BaseMaterial::updateDescriptorSets(const vk::Device device)
{
	for (auto descriptorSet : descriptorSets)
	{
		vk::DescriptorImageInfo albedoDescriptorImageInfo = {
			.sampler = albedo->getSampler(),
			.imageView = albedo->getImageView(),
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};

		vk::WriteDescriptorSet writeDescriptorSet = {
			.dstSet = descriptorSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.pImageInfo = &albedoDescriptorImageInfo
		};

		device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}
}

void BaseMaterial::createDescriptorSetLayout(const vk::Device device)
{
	const vk::DescriptorSetLayoutBinding albedoLayoutBinding = {
		.binding = 0,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eFragment
	};

	std::array<vk::DescriptorSetLayoutBinding, 1> layoutBindings
		= {albedoLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
		.pBindings = layoutBindings.data()
	};

	descriptorSetLayout = device.createDescriptorSetLayout(
		descriptorSetLayoutCreateInfo);
}

void BaseMaterial::createDescriptorSets(const vk::Device device)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts
		(1, descriptorSetLayout);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = descriptorSetLayouts.data()
	};

	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

void BaseMaterial::release(const vk::Device device)
{
	Material::release(device);
}
