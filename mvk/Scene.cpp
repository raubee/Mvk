#include "Scene.h"
#include "UniformBufferObject.h"

using namespace mvk;

void Scene::setup(const Device device, const uint32_t size,
                  const vk::Extent2D extent)
{
	createUniformBufferObject(device);
	updateUniformBufferObject(device, 0.0f, extent);
	createDescriptorSetLayout(device);
	createDescriptorPool(device, size);
	createDescriptorSets(device, size);
	updateDescriptorSets(device);
}

void Scene::update(const Device device, const float time,
                   const vk::Extent2D extent) const
{
	updateUniformBufferObject(device, time, extent);
}

void Scene::release(const Device device) const
{
	device.device.destroyDescriptorPool(descriptorPool);
	device.device.destroyDescriptorSetLayout(descriptorSetLayout);
	device.destroyBuffer(uniformBuffer);
}

void Scene::createUniformBufferObject(const Device device)
{
	const auto size = sizeof(UniformBufferObject);

	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eUniformBuffer,
	};

	uniformBuffer =
		alloc::allocateCpuToGpuBuffer(vma::Allocator(device), bufferCreateInfo);
}

void Scene::createDescriptorPool(const Device device, const uint32_t size)
{
	std::array<vk::DescriptorPoolSize, 1> descriptorPoolSizes{};
	descriptorPoolSizes[0].type = vk::DescriptorType::eUniformBuffer;
	descriptorPoolSizes[0].descriptorCount = size;

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo =
	{
		.maxSets = size,
		.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
		.pPoolSizes = descriptorPoolSizes.data()
	};

	descriptorPool = vk::Device(device)
		.createDescriptorPool(descriptorPoolCreateInfo);
}

void Scene::createDescriptorSets(const Device device, const uint32_t size)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts
		(size, descriptorSetLayout);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = size,
		.pSetLayouts = descriptorSetLayouts.data()
	};

	descriptorSets =
		vk::Device(device).allocateDescriptorSets(descriptorSetAllocateInfo);
}

void Scene::updateDescriptorSets(const Device device)
{
	for (auto descriptorSet : descriptorSets)
	{
		const vk::DescriptorBufferInfo descriptorBufferInfo =
		{
			.buffer = uniformBuffer.buffer,
			.offset = 0,
			.range = sizeof(UniformBufferObject)
		};

		const vk::WriteDescriptorSet writeDescriptorSet =
		{
			.dstSet = descriptorSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &descriptorBufferInfo,
		};

		vk::Device(device)
			.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}
}

void Scene::updateUniformBufferObject(const Device device, const float time,
                                      const vk::Extent2D extent) const
{
	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), 0.f,
	                        glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.view = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f),
	                       glm::vec3(0.0f, .75f, 0.0f),
	                       glm::vec3(0.0f, 1.0f, 0.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), extent.width /
	                            static_cast<float>(extent.height),
	                            0.1f, 100.0f);
	ubo.proj[1][1] *= -1;

	mapDataToBuffer(vma::Allocator(device), uniformBuffer, &ubo, sizeof ubo);
}

