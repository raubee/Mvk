#include "BaseMaterial.h"

using namespace mvk;

BaseMaterial::BaseMaterial() : Material("shaders/vert.spv", "shaders/frag.spv")
{
}

void BaseMaterial::load(const vma::Allocator allocator,
                        const vk::Device device,
                        const vk::CommandPool commandPool,
                        const vk::Queue transferQueue)
{
	Material::load(allocator, device, commandPool, transferQueue);

	const auto albedoName = description.albedo.c_str();

	albedo.load(
		  allocator, device, commandPool, transferQueue, albedoName);
}

void BaseMaterial::writeDescriptorSet(const vk::DescriptorSet descriptorSet)
{
	Material::writeDescriptorSet(descriptorSet);

	vk::DescriptorImageInfo albedoDescriptorImageInfo = {
		.sampler = albedo.getSampler() ,
		.imageView = albedo.getImageView(),
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

void BaseMaterial::release()
{
	Material::release();
	albedo.release();
}
