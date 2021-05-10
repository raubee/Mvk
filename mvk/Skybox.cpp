#include "Skybox.h"

using namespace mvk;

void Skybox::create(Device* device, const vk::Queue transferQueue,
                    const vk::RenderPass renderPass,
                    const std::array<std::string, 6> texturePaths)
{
	this->ptrDevice = device;

	loadShaders();
	loadCubemap(transferQueue, texturePaths);
	createDescriptorLayout(device);
	buildPipeline(renderPass);
	createUniformBufferObject(transferQueue);
	createSkyboxVertexBuffer(transferQueue);
	createDescriptorPool();
	createDescriptorSets();
	updateDescriptorSets();
}

void Skybox::loadShaders()
{
	vertexShader = new Shader(ptrDevice, "shaders/skybox.vert.spv",
	                          vk::ShaderStageFlagBits::eVertex);

	fragmentShader = new Shader(ptrDevice, "shaders/skybox.frag.spv",
	                            vk::ShaderStageFlagBits::eFragment);
}

void Skybox::loadCubemap(const vk::Queue transferQueue,
                         const std::array<std::string, 6> texturePaths)
{
	cubemap.loadFromSixFiles(ptrDevice, transferQueue, texturePaths,
	                         vk::Format::eR8G8B8A8Unorm);
}

void Skybox::createUniformBufferObject(vk::Queue transferQueue)
{
	const auto size = sizeof(UniformBufferObject);

	const vk::BufferCreateInfo bufferCreateInfo{
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eUniformBuffer,
	};

	uniformBuffer =
		alloc::allocateCpuToGpuBuffer(ptrDevice->allocator, bufferCreateInfo);
}

void Skybox::createSkyboxVertexBuffer(const vk::Queue transferQueue)
{
	std::vector<SkyboxVertex> vertices
	{
		// -Z
		{{-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
		{{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
		{{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f}},
		{{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f}},

		// +Z
		{{-1.0f, -1.0f, 1.0f}, {1.0f, 1.0f}},
		{{1.0f, -1.0f, 1.0f}, {0.0f, 1.0f}},
		{{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}
	};

	std::vector<uint16_t> indices
	{
		0, 1, 3, 3, 1, 2,
		1, 5, 2, 2, 5, 6,
		5, 4, 6, 6, 4, 7,
		4, 0, 7, 7, 0, 3,
		3, 2, 7, 7, 2, 6,
		4, 5, 0, 0, 5, 1
	};

	vertexCount = static_cast<uint32_t>(vertices.size());
	indexCount = static_cast<uint32_t>(indices.size());

	const auto vSize =
		static_cast<vk::DeviceSize>(sizeof vertices.at(0) * vertexCount);

	const auto iSize =
		static_cast<vk::DeviceSize>(sizeof indices.at(0) * indexCount);

	vertexBuffer = ptrDevice->transferDataSetToGpuBuffer(transferQueue,
	                                                     vertices.data(), vSize,
	                                                     vk::BufferUsageFlagBits
	                                                     ::eVertexBuffer);

	indexBuffer = ptrDevice->transferDataSetToGpuBuffer(transferQueue,
	                                                    indices.data(), iSize,
	                                                    vk::BufferUsageFlagBits
	                                                    ::eIndexBuffer);
}

void Skybox::createDescriptorLayout(Device* device)
{
	const std::vector<vk::DescriptorSetLayoutBinding> layoutBindings
	{
		// UBO
		{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eVertex
		},

		// Cubemap
		{
			.binding = 1,
			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1,
			.stageFlags = vk::ShaderStageFlagBits::eFragment
		}
	};

	const vk::DescriptorSetLayoutCreateInfo createInfo{
		.bindingCount = static_cast<uint32_t>(layoutBindings.size()),
		.pBindings = layoutBindings.data()
	};

	descriptorSetLayout =
		device->logicalDevice.createDescriptorSetLayout(createInfo);
}

void Skybox::createDescriptorPool()
{
	const std::vector<vk::DescriptorPoolSize> descriptorPoolSizes
	{
		{
			.type = vk::DescriptorType::eUniformBuffer,
			.descriptorCount = 1
		},
		{
			.type = vk::DescriptorType::eCombinedImageSampler,
			.descriptorCount = 1
		}
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
		.pPoolSizes = descriptorPoolSizes.data()
	};

	descriptorPool = ptrDevice->logicalDevice
	                          .createDescriptorPool(descriptorPoolCreateInfo);
}

void Skybox::createDescriptorSets()
{
	const vk::DescriptorSetAllocateInfo allocateInfo{
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSetLayout
	};

	descriptorSets =
		ptrDevice->logicalDevice.allocateDescriptorSets(allocateInfo);
}

void Skybox::updateDescriptorSets()
{
	for (const auto& descriptorSet : descriptorSets)
	{
		// UBO
		const vk::DescriptorBufferInfo bufferInfo{
			.buffer = uniformBuffer.buffer,
			.offset = {0},
			.range = sizeof(UniformBufferObject)
		};

		// Cubemap
		const vk::DescriptorImageInfo imageInfo{
			.sampler = cubemap.getSampler(),
			.imageView = cubemap.getImageView(),
			.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		};

		const std::vector<vk::WriteDescriptorSet> writeDescriptorSets
		{
			{
				.dstSet = descriptorSet,
				.dstBinding = 0,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo = &bufferInfo
			},
			{
				.dstSet = descriptorSet,
				.dstBinding = 1,
				.dstArrayElement = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo
			}
		};

		ptrDevice->logicalDevice.updateDescriptorSets(
			static_cast<uint32_t>(writeDescriptorSets.size()),
			writeDescriptorSets.data(), 0, nullptr);
	}
}

void Skybox::buildPipeline(const vk::RenderPass renderPass)
{
	const std::vector<vk::PipelineShaderStageCreateInfo> shaderStageCreateInfos
	{
		vertexShader->getPipelineShaderCreateInfo(),
		fragmentShader->getPipelineShaderCreateInfo()
	};

	const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{
		descriptorSetLayout
	};

	const std::vector<vk::VertexInputBindingDescription>
		vertexInputBindingDescription{
			SkyboxVertex::getVertexInputBindingDescription()
		};

	const auto& vertexInputAttributeDescriptions =
		SkyboxVertex::getVertexInputAttributeDescriptions();

	const GraphicPipelineCreateInfo createInfo{
		.vertexInputBindingDescription = vertexInputBindingDescription,
		.vertexInputAttributeDescription = vertexInputAttributeDescriptions,
		.renderPass = renderPass,
		.shaderStageCreateInfos = shaderStageCreateInfos,
		.descriptorSetLayouts = descriptorSetLayouts,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.alpha = false,
		.depthTest = false
	};

	graphicPipeline.build(ptrDevice, createInfo);
}

void Skybox::release() const
{
	cubemap.release();

	vertexShader->release();
	fragmentShader->release();

	graphicPipeline.release();

	ptrDevice->logicalDevice.destroyDescriptorPool(descriptorPool);
	ptrDevice->logicalDevice.destroyDescriptorSetLayout(descriptorSetLayout);

	ptrDevice->destroyBuffer(uniformBuffer);
	ptrDevice->destroyBuffer(vertexBuffer);
	ptrDevice->destroyBuffer(indexBuffer);
}
