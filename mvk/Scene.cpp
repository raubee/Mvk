#include "Scene.h"

using namespace mvk;

void Scene::setup(Device* device)
{
	this->ptrDevice = device;

	modelMatrix = glm::rotate(glm::mat4(1.0f), 0.0f,
	                          glm::vec3(0.0f, 1.0f, 0.0f));

	createUniformBufferObject();
	updateUniformBufferObject(0.0f);
	createDescriptorSetLayout(device);
	createDescriptorPool();
	createDescriptorSets();
	updateDescriptorSets();
}

void Scene::setupSkybox(const vk::Queue transferQueue,
                        const vk::RenderPass renderPass,
                        const std::array<std::string, 6> texturePaths)
{
	skybox = new Skybox();
	skybox->create(ptrDevice, transferQueue, renderPass, texturePaths);
}

void Scene::update(const float time) const
{
	updateUniformBufferObject(time);
}

void Scene::release() const
{
	if (skybox)
		skybox->release();

	ptrDevice->destroyBuffer(uniformBuffer);
	ptrDevice->logicalDevice.destroyDescriptorPool(descriptorPool);

	if (descriptorSetLayout)
		ptrDevice->logicalDevice
		         .destroyDescriptorSetLayout(descriptorSetLayout);
}

void Scene::renderSkybox(const vk::CommandBuffer commandBuffer) const
{
	if (!skybox)
	{
		return;
	}

	const auto pipeline = skybox->graphicPipeline.getPipeline();
	const auto pipelineLayout = skybox->graphicPipeline.getPipelineLayout();

	const std::vector<vk::DescriptorSet> descriptorSets = {
		skybox->getDescriptorSet(0)
	};

	const auto descriptorCount =
		static_cast<uint32_t>(descriptorSets.size());

	const auto vertexBuffer = skybox->vertexBuffer;
	const auto indexBuffer = skybox->indexBuffer;
	const auto indexCount = skybox->indexCount;

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
	                                 pipelineLayout, 0, descriptorCount,
	                                 descriptorSets.data(), 0, nullptr);

	vk::DeviceSize offsets[] = {0};

	commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer, offsets);
	commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
	                              vk::IndexType::eUint16);
	commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
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

void Scene::createDescriptorPool()
{
	vk::DescriptorPoolSize descriptorPoolSizes{
		.type = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 1
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo =
	{
		.maxSets = 1,
		.poolSizeCount = 1,
		.pPoolSizes = &descriptorPoolSizes
	};

	descriptorPool = ptrDevice->logicalDevice
	                          .createDescriptorPool(descriptorPoolCreateInfo);
}

void Scene::createDescriptorSets()
{
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSetLayout
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

void Scene::updateUniformBufferObject(const float time) const
{
	UniformBufferObject ubo{};

	ubo.model = modelMatrix;
	ubo.view = camera.viewMatrix;
	ubo.proj = camera.projMatrix;
	ubo.proj[1][1] *= -1;
	ubo.camPos = camera.position;

	mapDataToBuffer(ptrDevice->allocator, uniformBuffer, &ubo, sizeof ubo);

	if (skybox)
	{
		Skybox::UniformBufferObject uboS{};

		uboS.model = modelMatrix;
		uboS.view = camera.viewMatrix;
		uboS.view[3] = glm::vec4(0, 0, 0, 1);
		uboS.proj = camera.projMatrix;
		uboS.proj[1][1] *= -1;

		mapDataToBuffer(ptrDevice->allocator, skybox->uniformBuffer, &uboS,
		                sizeof uboS);
	}
}
