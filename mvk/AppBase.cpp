#include "AppBase.h"
#include <set>
#include <iostream>

using namespace mvk;

static alloc::Buffer createGpuVertexBuffer(vma::Allocator allocator,
                                           vk::DeviceSize size);
static alloc::Buffer createGpuIndexBuffer(vma::Allocator allocator,
                                          vk::DeviceSize size);

static void
framebufferResizeCallback(GLFWwindow* window, int width, int height);

AppBase::AppBase()
	: needResize(false)
{
	const auto appName = "Test";

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(600, 600, appName, nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	uint32_t glfwExtensionsCount = 0;
	const auto glfwExtensions = glfwGetRequiredInstanceExtensions(
		&glfwExtensionsCount);

	mvk::ContextCreationInfo createInfo;

#if (NDEBUG)
	createInfo.addInstanceLayer("VK_LAYER_KHRONOS_validation");
	createInfo.addInstanceLayer("VK_LAYER_LUNARG_monitor");
	createInfo.addInstanceLayer("VK_LAYER_LUNARG_standard_validation");
#endif

	for (size_t i = 0; i < glfwExtensionsCount; i++)
	{
		createInfo.addInstanceExtension(glfwExtensions[i]);
	}

	createInfo.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	context.createInstance(createInfo);

	surface = getGlfwSurfaceKHR(context, window);

	context.pickPhysicalDevice(createInfo);
	context.createDevice(createInfo);

	this->instance = context.getInstance();
	this->physicalDevice = context.getPhysicalDevice();
	this->device = context.getDevice();
	this->graphicQueue = context.getGraphicsQueue();
	this->presentQueue = context.getPresentQueue();
	this->transferQueue = context.getTransferQueue();
	this->surface = surface;

	if (!physicalDevice.getSurfaceSupportKHR(
		context.getGraphicsQueueFamilyIndex(), surface))
	{
		throw std::runtime_error("AppBase: Surface KHR not supported!");
	}

	if (!checkSwapchainSupport())
	{
		throw std::runtime_error("AppBase: Swapchain KHR can't be created!");
	}

	allocator = mvk::alloc::init(physicalDevice, device, instance);

	createSemaphores();
	createCommandPool(context.getGraphicsQueueFamilyIndex());
	updateSwapchain();

	startTime = std::chrono::high_resolution_clock::now();

	const auto swapchainSize =
		static_cast<uint32_t>(swapchain.getSwapchainSwainSize());
	const auto swapchainExtent = swapchain.getSwapchainExtent();
	scene.init(device, allocator, swapchainSize, swapchainExtent);

	glfwSetWindowUserPointer(window, this);
}

void AppBase::release()
{
	scene.release(device, allocator);

	cleanupSwapchain();

	device.destroyDescriptorPool(descriptorPool);
	device.destroyCommandPool(commandPool);
	device.destroySemaphore(imageAvailableSemaphore);
	device.destroySemaphore(renderFinishedSemaphore);
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
	update();

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

void AppBase::setSwapchainDirty()
{
	needResize = true;
}

void AppBase::run()
{
	while (!glfwWindowShouldClose(window))
	{
		drawFrame();
		glfwPollEvents();
	}
}

void AppBase::terminate()
{
	waitIdle();
	release();

	context.getInstance().destroySurfaceKHR(surface);
	context.release();

	glfwDestroyWindow(window);
	glfwTerminate();
}

vk::SurfaceKHR AppBase::getGlfwSurfaceKHR(const mvk::Context context,
                                          GLFWwindow* window)
{
	VkSurfaceKHR _surface;

	if (glfwCreateWindowSurface(
		static_cast<VkInstance>(context.getInstance()),
		window,
		nullptr,
		&_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface khr");
	}

	const auto surface = vk::SurfaceKHR(_surface);
	return surface;
}

void AppBase::updateSwapchain()
{
	device.waitIdle();

	cleanupSwapchain();
	createSwapchain();
	createRenderPass();
	createSwapchainFrames();

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
	swapchain.create(physicalDevice, device, allocator, commandPool,
	                 transferQueue, surface);
}

void AppBase::createRenderPass()
{
	const auto swapchainFormat = swapchain.getSwapchainFormat();
	const auto depthFormat = swapchain.getDepthFormat();

	renderPass.create(device, swapchainFormat, depthFormat);
}

void AppBase::createSwapchainFrames()
{
	swapchain.createSwapchainFrames(device, renderPass.getRenderPass());
}

void AppBase::createCommandPool(const uint32_t queueFamily)
{
	const vk::CommandPoolCreateInfo commandPoolCreateInfo = {
		.queueFamilyIndex = queueFamily
	};

	commandPool = device.createCommandPool(commandPoolCreateInfo);
}

void AppBase::setupCommandBuffers()
{
	swapchain.createCommandBuffers(device, commandPool);
	auto frames = swapchain.getSwapchainFrames();

	for (auto i = 0; i < frames.size(); i++)
	{
		setupFrameCommandBuffer(i, frames[i]);
	}
}

void AppBase::setupFrameCommandBuffer(const int index,
                                      const SwapchainFrame swapchainFrame)
{
	const auto commandBuffer = swapchainFrame.getCommandBuffer();
	const auto framebuffer = swapchainFrame.getFramebuffer();
	const auto swapchainExtent = swapchain.getSwapchainExtent();

	const vk::CommandBufferBeginInfo commandBufferBeginInfo{};
	commandBuffer.begin(commandBufferBeginInfo);

	const std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	std::array<vk::ClearValue, 2> clearValues{};
	clearValues[0].setColor(clearColor);
	clearValues[1].setDepthStencil({1.0f, 0});

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

	const auto objects = scene.getObjects();

	for (auto object : objects)
	{
		const auto geometry = object->geometry;
		const auto material = object->material;
		const auto graphicPipeline = object->graphicPipeline;
		const auto pipelineLayout = graphicPipeline->getPipelineLayout();
		const auto pipeline = graphicPipeline->getPipeline();

		std::vector<vk::DescriptorSet> descriptorSets = {
			scene.getDescriptorSet(0)
		};

		const auto matDescriptorSet = material->getDescriptorSet(0);

		if (matDescriptorSet)
		{
			descriptorSets.push_back(matDescriptorSet);
		}

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		                           pipeline);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		                                 pipelineLayout, 0,
		                                 static_cast<uint32_t>(
			                                 descriptorSets.size()),
		                                 descriptorSets.data(), 0, nullptr);

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

		const auto vertexBuffer = geometry->getVertexBuffer();
		const auto indexBuffer = geometry->getIndexBuffer();

		vk::DeviceSize offsets[] = {0};

		commandBuffer.bindVertexBuffers(0, 1,
		                                &vertexBuffer.buffer,
		                                offsets);

		if (indexBuffer.buffer)
		{
			const auto size = geometry->getIndexCount();

			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
			                              vk::IndexType::eUint16);

			commandBuffer.drawIndexed(size, 1, 0, 0, 0);
		}
	}

	commandBuffer.endRenderPass();
	commandBuffer.end();
}

void AppBase::createSemaphores()
{
	const vk::SemaphoreCreateInfo semaphoreCreateInfo;
	imageAvailableSemaphore = device.createSemaphore(semaphoreCreateInfo);
	renderFinishedSemaphore = device.createSemaphore(semaphoreCreateInfo);
}

void AppBase::cleanupSwapchain()
{
	swapchain.release(device, allocator);
	renderPass.release(device);
}

void AppBase::update()
{
	const auto currentTime = std::chrono::high_resolution_clock::now();
	const auto time = std::chrono::duration<float, std::chrono::seconds::period>
		(currentTime - startTime).count();

	scene.update(allocator, time, swapchain.getSwapchainExtent());
}

alloc::Buffer AppBase::createVertexBufferObject(
	std::vector<Vertex> vertices) const
{
	const auto size =
		static_cast<vk::DeviceSize>(sizeof vertices.at(0) * vertices.
			size());
	const auto stagingVertexBuffer =
		alloc::allocateStagingTransferBuffer(allocator, vertices.data(), size);
	const auto vertexBuffer = createGpuVertexBuffer(allocator, size);
	copyCpuToGpuBuffer(device, commandPool, transferQueue,
	                   stagingVertexBuffer, vertexBuffer,
	                   size);
	alloc::deallocateBuffer(allocator, stagingVertexBuffer);

	return vertexBuffer;
}

alloc::Buffer AppBase::createIndexBufferObject(
	std::vector<uint16_t> indices) const
{
	const auto size =
		static_cast<vk::DeviceSize>(sizeof indices.at(0) * indices.
			size());
	const auto stagingVertexBuffer =
		alloc::allocateStagingTransferBuffer(allocator, indices.data(), size);
	const auto indexBuffer = createGpuIndexBuffer(allocator, size);
	alloc::copyCpuToGpuBuffer(device, commandPool, transferQueue,
	                          stagingVertexBuffer, indexBuffer, size);
	alloc::deallocateBuffer(allocator, stagingVertexBuffer);

	return indexBuffer;
}

alloc::Image AppBase::createTextureBufferObject(unsigned char* pixels,
                                                const uint32_t width,
                                                const uint32_t height,
                                                const vk::Format format) const
{
	const vk::DeviceSize imageSize = width * height * 4;
	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = imageSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};

	const auto stagingBuffer =
		alloc::allocateMappedCpuToGpuBuffer(allocator, bufferCreateInfo,
		                                    pixels);

	const vk::Extent3D imageExtent = {
		.width = width,
		.height = height,
		.depth = 1
	};

	const vk::ImageCreateInfo imageCreateInfo = {
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = imageExtent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage = vk::ImageUsageFlagBits::eTransferDst |
		vk::ImageUsageFlagBits::eSampled,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined,
	};

	const auto imageBuffer = alloc::allocateGpuOnlyImage(allocator,
	                                                     imageCreateInfo);

	const vk::BufferImageCopy bufferImageCopy = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {0, 0},
		.imageExtent = imageExtent,
	};

	alloc::transitionImageLayout(device, commandPool, graphicQueue,
	                             imageBuffer.image, format,
	                             vk::ImageLayout::eUndefined,
	                             vk::ImageLayout::eTransferDstOptimal);

	alloc::copyCpuBufferToGpuImage(device, commandPool, graphicQueue,
	                               stagingBuffer, imageBuffer, bufferImageCopy);

	alloc::transitionImageLayout(device, commandPool, graphicQueue,
	                             imageBuffer.image, format,
	                             vk::ImageLayout::eTransferDstOptimal,
	                             vk::ImageLayout::eShaderReadOnlyOptimal);

	deallocateBuffer(allocator, stagingBuffer);

	return imageBuffer;
}

static alloc::Buffer createGpuVertexBuffer(const vma::Allocator allocator,
                                           const vk::DeviceSize size)
{
	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eVertexBuffer,
		.sharingMode = vk::SharingMode::eExclusive
	};

	const auto buffer =
		alloc::allocateGpuOnlyBuffer(allocator, bufferCreateInfo);

	return buffer;
}

static alloc::Buffer createGpuIndexBuffer(const vma::Allocator allocator,
                                          const vk::DeviceSize size)
{
	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eIndexBuffer,
		.sharingMode = vk::SharingMode::eExclusive
	};

	const auto buffer =
		alloc::allocateGpuOnlyBuffer(allocator, bufferCreateInfo);

	return buffer;
}

void framebufferResizeCallback(GLFWwindow* window, int width,
                               int height)
{
	const auto app =
		reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window));

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	app->setSwapchainDirty();
}