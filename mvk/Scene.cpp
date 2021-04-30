#include "Scene.h"
#include "UniformBufferObject.h"

using namespace mvk;

void Scene::setup(const vk::Device device,
                  const vma::Allocator allocator,
                  const uint32_t size,
                  const vk::Extent2D extent)
{
	createUniformBufferObject(allocator);
	updateUniformBufferObject(allocator, 0.0f, extent);
	createDescriptorSetLayout(device);
	createDescriptorPool(device, size);
	createDescriptorSets(device, size);
	updateDescriptorSets(device);
}

void Scene::update(const vma::Allocator allocator, const float time,
                   const vk::Extent2D extent) const
{
	updateUniformBufferObject(allocator, time, extent);
}

void Scene::release(const vk::Device device,
                    const vma::Allocator allocator) const
{
	device.destroyDescriptorPool(descriptorPool);
	device.destroyDescriptorSetLayout(descriptorSetLayout);
	alloc::deallocateBuffer(allocator, uniformBuffer);
}

void Scene::createDescriptorSetLayout(const vk::Device device)
{
	const vk::DescriptorSetLayoutBinding uniformBufferLayoutBinding = {
		.binding = 0,
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eVertex
	};

	std::array<vk::DescriptorSetLayoutBinding, 1> layoutBindings
		= {uniformBufferLayoutBinding};

	const vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
		.pBindings = layoutBindings.data()
	};

	descriptorSetLayout = device.createDescriptorSetLayout(
		descriptorSetLayoutCreateInfo);
}

void Scene::createUniformBufferObject(const vma::Allocator allocator)
{
	const auto size = sizeof(UniformBufferObject);

	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eUniformBuffer,
	};

	uniformBuffer = alloc::allocateCpuToGpuBuffer(allocator, bufferCreateInfo);
}

void Scene::createDescriptorPool(const vk::Device device, const uint32_t size)
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

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);
}

void Scene::createDescriptorSets(const vk::Device device, const uint32_t size)
{
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts
		(size, descriptorSetLayout);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = size,
		.pSetLayouts = descriptorSetLayouts.data()
	};

	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
}

void Scene::updateDescriptorSets(const vk::Device device)
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

		device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}
}

void Scene::updateUniformBufferObject(const vma::Allocator allocator,
                                      const float time,
                                      const vk::Extent2D extent) const
{
	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
	                        glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 1.0f),
	                       glm::vec3(0.0f, 0.0f, 0.75f),
	                       glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), extent.width /
	                            static_cast<float>(extent.height),
	                            0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	mapDataToBuffer(allocator, uniformBuffer, &ubo, sizeof ubo);
}
