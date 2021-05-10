#include "BaseMaterial.h"

using namespace mvk;

void BaseMaterial::load(Device* device,
                        const BaseMaterialDescription description)

{
	alphaMode = description.alphaMode;
	baseColor = description.baseColor;
	normal = description.normal;
	metallicRoughness = description.metallicRoughness;

	constants = description.constants;

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

void BaseMaterial::createDescriptorSetLayout(Device* device)
{
	std::vector<vk::DescriptorSetLayoutBinding> layoutBindings
	{

		// BaseColor
		{
			0, vk::DescriptorType::eCombinedImageSampler, 1,
			vk::ShaderStageFlagBits::eFragment
		},

		// Normal
		{
			1, vk::DescriptorType::eCombinedImageSampler, 1,
			vk::ShaderStageFlagBits::eFragment
		},

		// MetallicRoughness
		{
			2, vk::DescriptorType::eCombinedImageSampler, 1,
			vk::ShaderStageFlagBits::eFragment
		},

	};

	const auto bindingCount =
		static_cast<uint32_t>(layoutBindings.size());

	const vk::DescriptorSetLayoutCreateInfo
		descriptorSetLayoutCreateInfo{
			.bindingCount = bindingCount,
			.pBindings = layoutBindings.data()
		};

	descriptorSetLayout = device->logicalDevice
	                            .createDescriptorSetLayout(
		                            descriptorSetLayoutCreateInfo);
}

void BaseMaterial::createDescriptorPool()
{
	const vk::DescriptorPoolSize descriptorPoolSize{
		.type = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = 1
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{
		.maxSets = 1,
		.poolSizeCount = 1,
		.pPoolSizes = &descriptorPoolSize
	};

	descriptorPool = ptrDevice->logicalDevice
	                          .createDescriptorPool(descriptorPoolCreateInfo);
}

void BaseMaterial::createDescriptorSets()
{
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSetLayout
	};

	descriptorSets = ptrDevice->logicalDevice
	                          .allocateDescriptorSets(
		                          descriptorSetAllocateInfo);
}

void BaseMaterial::updateDescriptorSets()
{
	for (const auto& descriptorSet : descriptorSets)
	{
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets{

			// Base color
			{
				.dstSet = descriptorSet,
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &baseColor->descriptorInfo
			},

			// Normal
			{
				.dstSet = descriptorSet,
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &normal->descriptorInfo
			},

			// Metallic - Roughness
			{
				.dstSet = descriptorSet,
				.dstBinding = 2,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &metallicRoughness->descriptorInfo
			}
		};

		const auto size = static_cast<uint32_t>(writeDescriptorSets.size());

		ptrDevice->logicalDevice
		         .updateDescriptorSets(size, writeDescriptorSets.data(), 0,
		                               nullptr);
	}
}
