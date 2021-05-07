#include "Scene.h"

using namespace mvk;

void Scene::setup(Device* device, const uint32_t size,
                  const vk::Extent2D extent)
{
	this->ptrDevice = device;

	modelMatrix = glm::rotate(glm::mat4(1.0f), 0.0f,
		glm::vec3(0.0f, 1.0f, 0.0f));

	createUniformBufferObject();
	updateUniformBufferObject(0.0f, extent);
	createDescriptorSetLayout(device);
	createDescriptorPool(size);
	createDescriptorSets(size);
	updateDescriptorSets();
}

void Scene::update(const float time, const vk::Extent2D extent) const
{
	updateUniformBufferObject(time, extent);
}

void Scene::release() const
{
	ptrDevice->destroyBuffer(uniformBuffer);
	ptrDevice->logicalDevice.destroyDescriptorPool(descriptorPool);

	if (descriptorSetLayout)
		ptrDevice->logicalDevice
		         .destroyDescriptorSetLayout(descriptorSetLayout);
}

void Scene::createUniformBufferObject()
{
	const auto size = sizeof(UniformBufferObject);

	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eUniformBuffer,
	};

	uniformBuffer =
		alloc::allocateCpuToGpuBuffer(ptrDevice->allocator, bufferCreateInfo);
}

void Scene::createDescriptorPool(const uint32_t size)
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

	descriptorPool = ptrDevice->logicalDevice
	                          .createDescriptorPool(descriptorPoolCreateInfo);
}

void Scene::createDescriptorSets(const uint32_t size)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts
		(size, descriptorSetLayout);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = size,
		.pSetLayouts = descriptorSetLayouts.data()
	};

	descriptorSets = ptrDevice->logicalDevice
	                          .allocateDescriptorSets(
		                          descriptorSetAllocateInfo);
}

void Scene::updateDescriptorSets()
{
	for (const auto& descriptorSet : descriptorSets)
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

		ptrDevice->logicalDevice
		      .updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}
}

void Scene::updateUniformBufferObject(const float time,
                                      const vk::Extent2D extent) const
{
	UniformBufferObject ubo{};
	ubo.model = modelMatrix;
	ubo.view = camera.viewMatrix;
	ubo.proj = camera.projMatrix;
	ubo.proj[1][1] *= -1;
	ubo.camPos = camera.position;

	mapDataToBuffer(ptrDevice->allocator, uniformBuffer, &ubo, sizeof ubo);
}
