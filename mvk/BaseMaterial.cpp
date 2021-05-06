#include "BaseMaterial.h"

using namespace mvk;

void BaseMaterial::load(Device* device,
                        const BaseMaterialDescription description)

{
	albedo = description.albedo;

	Material::load(device,
	               new Shader(device, "shaders/base.vert.spv",
	                          vk::ShaderStageFlagBits::eVertex),
	               new Shader(device, "shaders/base.frag.spv",
	                          vk::ShaderStageFlagBits::eFragment));

	if (!descriptorSetLayout)
		createDescriptorSetLayout(device);

	createDescriptorPool();
	createDescriptorSets();
	updateDescriptorSets();
}

void BaseMaterial::release()
{
	ptrDevice->logicalDevice.destroyDescriptorPool(descriptorPool);

	// Fix: static destroy
	if (descriptorSetLayout)
		ptrDevice->logicalDevice
		         .destroyDescriptorSetLayout(descriptorSetLayout);

	descriptorSetLayout = nullptr;

	Material::release();
}

void BaseMaterial::createDescriptorPool()
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

	descriptorPool = ptrDevice->logicalDevice
	                          .createDescriptorPool(descriptorPoolCreateInfo);
}

void BaseMaterial::createDescriptorSets()
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts
		(1, descriptorSetLayout);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = descriptorSetLayouts.data()
	};

	descriptorSets = ptrDevice->logicalDevice
	                          .allocateDescriptorSets(
		                          descriptorSetAllocateInfo);
}

void BaseMaterial::updateDescriptorSets()
{
	for (const auto& descriptorSet : descriptorSets)
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

		ptrDevice->logicalDevice
		         .updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}
}
