#include "AppBase.h"
#include "UniformBufferObject.h"
#include <set>
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace mvk;

void AppBase::setup(const Context context, const vk::SurfaceKHR surface)
{
	this->instance = context.getInstance();
	this->physicalDevice = context.getPhysicalDevice();
	this->device = context.getDevice();
	this->graphicQueue = context.getGraphicsQueue();
	this->presentQueue = context.getPresentQueue();
	this->transferQueue = context.getTransferQueue();
	this->surface = surface;

	allocator = createVmaAllocator(context);

	if (!physicalDevice.getSurfaceSupportKHR(
		context.getGraphicsQueueFamilyIndex(), surface))
	{
		throw std::runtime_error("AppBase: Surface KHR not supported!");
	}

	if (!checkSwapchainSupport())
	{
		throw std::runtime_error("AppBase: Swapchain KHR can't be created!");
	}

	createSemaphores();
	createCommandPool(context.getGraphicsQueueFamilyIndex());

	auto objects = scene.getObjects();

	for (auto object : objects)
	{
		if (object != nullptr)
			object->load(allocator, device, commandPool, transferQueue);
	}

	updateSwapchain();

	startTime = std::chrono::high_resolution_clock::now();
}

void AppBase::release()
{
	for (auto object : scene.getObjects())
	{
		if (object != nullptr)
			object->release();
	}

	cleanupSwapchain();

	device.destroyCommandPool(commandPool);
	device.destroySemaphore(imageAvailableSemaphore);
	device.destroySemaphore(renderFinishedSemaphore);

	destroyVmaAllocator(allocator);
}

void AppBase::waitIdle() const
{
	device.waitIdle();
}

void AppBase::drawFrame()
{
	uint32_t imageIndex;
	vk::Result result;

	try
	{
		result = device.acquireNextImageKHR(swapchain.getSwapchain(),
		                                    UINT64_MAX,
		                                    imageAvailableSemaphore,
		                                    nullptr,
		                                    &imageIndex);
	}
	catch (vk::OutOfDateKHRError error)
	{
		updateSwapchain();
		return;
	}

	const auto currentSwapchainFrame = swapchain.getSwapchainFrame(imageIndex);
	updateUniformBuffer(currentSwapchainFrame);

	vk::Semaphore waitSemaphores[] = {imageAvailableSemaphore};
	vk::PipelineStageFlags waitStages[] = {
		vk::PipelineStageFlagBits::eColorAttachmentOutput
	};
	vk::Semaphore signalSemaphores[] = {renderFinishedSemaphore};

	const auto commandBuffer = currentSwapchainFrame.getCommandBuffer();

	const vk::SubmitInfo submitInfo = {
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores,
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores
	};

	result = graphicQueue.submit(1, &submitInfo, nullptr);

	vk::SwapchainKHR swapchains[] = {swapchain.getSwapchain()};
	const vk::PresentInfoKHR presentInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores,
		.swapchainCount = 1,
		.pSwapchains = swapchains,
		.pImageIndices = &imageIndex
	};

	try
	{
		result = graphicQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error)
	{
		updateSwapchain();
		return;
	}

	graphicQueue.waitIdle();
}

void AppBase::setFramebufferDirty()
{
	needResize = true;
}

void AppBase::updateSwapchain()
{
	device.waitIdle();

	cleanupSwapchain();

	createSwapchain();
	createRenderPass();
	createSwapchainFrames();
	createGraphicPipeline();
	createDescriptorSets();
	setupSwapchainFramesCommandBuffers();

	needResize = false;

	std::cout << "Swapchain updated!" << std::endl;
}

bool AppBase::checkSwapchainSupport()
{
	auto availableExtensions =
		physicalDevice.enumerateDeviceExtensionProperties(nullptr);

	std::set<std::string> requiredExtensions(deviceExtensions.begin(),
	                                         deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void AppBase::createSwapchain()
{
	swapchain.init(physicalDevice, device, commandPool, allocator,
	               transferQueue, surface);
}

void AppBase::createRenderPass()
{
	const auto swapchainFormat = swapchain.getSwapchainFormat();
	const auto depthFormat = swapchain.getDepthFormat();
	renderPass.init(device, swapchainFormat, depthFormat);
}

void AppBase::createSwapchainFrames()
{
	swapchain.createSwapchainFrames(renderPass.getRenderPass());
}

void AppBase::createGraphicPipeline()
{
	const auto extent = swapchain.getSwapchainExtent();
	graphicPipeline.init(device, renderPass, extent);
}

void AppBase::createDescriptorSets()
{
	const auto descriptorSetLayout = graphicPipeline.getDescriptorSetLayout();
	swapchain.createDescriptorSets(descriptorSetLayout);

	auto objects = scene.getObjects();

	for (auto swapchainFrame : swapchain.getSwapchainFrames())
	{
		const auto descriptorSet = swapchainFrame.getDescriptorSet();

		for (auto object : objects)
		{
			if (object != nullptr)
				object->writeDescriptorSet(descriptorSet);
		}
	}
}

void AppBase::createCommandPool(const uint32_t graphicsQueueFamilyIndex)
{
	const vk::CommandPoolCreateInfo commandPoolCreateInfo = {
		.queueFamilyIndex = graphicsQueueFamilyIndex
	};

	commandPool = device.createCommandPool(commandPoolCreateInfo);
}

void AppBase::setupSwapchainFramesCommandBuffers()
{
	swapchain.createSwapchainFramesCommandBuffers();

	for (const auto swapchainFrame : swapchain.getSwapchainFrames())
	{
		auto commandBuffer = swapchainFrame.getCommandBuffer();
		auto framebuffer = swapchainFrame.getFramebuffer();
		auto swapchainExtent = swapchain.getSwapchainExtent();

		auto j = 0;

		const vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBuffer.begin(commandBufferBeginInfo);

		const std::array<float,4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		std::array<vk::ClearValue, 2> clearValues{};
		clearValues[0].setColor(clearColor);
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		
		const vk::RenderPassBeginInfo renderPassBeginInfo = {
			.renderPass = renderPass.getRenderPass(),
			.framebuffer = framebuffer,
			.renderArea = {
				.offset = {0, 0},
				.extent = swapchainExtent
			},
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data()
		};

		commandBuffer.beginRenderPass(renderPassBeginInfo,
		                              vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		                           graphicPipeline.getPipeline());

		const auto descriptorSet = swapchainFrame.getDescriptorSet();
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		                                 graphicPipeline.
		                                 getPipelineLayout(), 0, 1,
		                                 &descriptorSet, 0, nullptr);

		vk::Viewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(swapchainExtent.width),
			.height = static_cast<float>(swapchainExtent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		commandBuffer.setViewport(0, viewport);

		vk::Rect2D scissor = {
			.offset = {0, 0},
			.extent = swapchainExtent
		};

		commandBuffer.setScissor(0, scissor);

		if (scene.getObjects().size() > 0)
		{
			const auto object = scene.getObjects().front();
			const auto vertexBuffers = object->geometry->getVertexBuffers();
			const auto indexBuffer = object->geometry->getIndexBuffer();

			if (vertexBuffers.size() > 0)
			{
				vk::DeviceSize offsets[] = {0};

				commandBuffer.bindVertexBuffers(0, 1,
				                                &vertexBuffers.at(0).buffer,
				                                offsets);
			}

			if (indexBuffer.buffer)
			{
				const auto size = object->geometry->getIndices()->size();

				commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
				                              vk::IndexType::eUint16);

				commandBuffer.drawIndexed(static_cast<uint32_t>(size), 1, 0, 0,
				                          0);
			}
		}

		commandBuffer.endRenderPass();
		commandBuffer.end();
		++j;
	}
}

void AppBase::createSemaphores()
{
	const vk::SemaphoreCreateInfo semaphoreCreateInfo;
	imageAvailableSemaphore = device.createSemaphore(semaphoreCreateInfo);
	renderFinishedSemaphore = device.createSemaphore(semaphoreCreateInfo);
}

void AppBase::updateUniformBuffer(const SwapchainFrame swapchainFrame)
{
	const auto currentTime = std::chrono::high_resolution_clock::now();
	const auto time = std::chrono::duration<float, std::chrono::seconds::period>
		(currentTime - startTime).count();

	const auto swapChainExtent = swapchain.getSwapchainExtent();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
	                        glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 1.0f),
	                       glm::vec3(0.0f, 0.0f, 0.75f),
	                       glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
	                            static_cast<float>(swapChainExtent.height),
	                            0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	const auto buffer = swapchainFrame.getUniformBuffer();
	mapDataToBuffer(allocator, buffer, &ubo, sizeof ubo);
}

void AppBase::cleanupSwapchain()
{
	swapchain.release();
	graphicPipeline.release();
	renderPass.release();
}
