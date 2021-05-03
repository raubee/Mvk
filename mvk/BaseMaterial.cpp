#include "BaseMaterial.h"

using namespace mvk;

void BaseMaterial::load(const Device device,
                        const BaseMaterialDescription description)

{
	albedo = description.albedo;

	Material::load(new Shader(device, "shaders/vert.spv",
	                          vk::ShaderStageFlagBits::eVertex),
	               new Shader(device, "shaders/frag.spv",
	                          vk::ShaderStageFlagBits::eFragment));

	createDescriptorSetLayout(device);
	createDescriptorPool(device);
	createDescriptorSets(device);
	updateDescriptorSets(device);
}

void BaseMaterial::release(const Device device)
{
	vk::Device(device).destroyDescriptorPool(descriptorPool);
	vk::Device(device).destroyDescriptorSetLayout(descriptorSetLayout);

	Material::release(device);
}

void BaseMaterial::createDescriptorPool(const Device device)
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

	descriptorPool =
		vk::Device(device).createDescriptorPool(descriptorPoolCreateInfo);
}

void BaseMaterial::createDescriptorSets(const Device device)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts
	(1, descriptorSetLayout);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = descriptorSetLayouts.data()
	};

	descriptorSets =
		vk::Device(device).allocateDescriptorSets(descriptorSetAllocateInfo);
}

void BaseMaterial::updateDescriptorSets(const Device device)
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

		vk::Device(device)
			.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}
}