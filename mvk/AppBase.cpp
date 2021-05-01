#include "AppBase.h"
#include <set>
#include <iostream>

using namespace mvk;

AppBase::AppBase(const AppInfo info)
	: appName(info.appName),
	  width(info.width),
	  height(info.height),
	  needResize(false),
	  startTime(std::chrono::high_resolution_clock::now())
{
	setupWindow(info.fullscreen);
	createInstance();
	createSurfaceKHR();
	pickPhysicalDevice();
	createDevice();
	createQueues();
	createSemaphores();
	updateSwapchain();
	setupScene();
}

AppBase::~AppBase()
{
	waitIdle();

	scene.release(device);
	renderPass.release(device);
	swapchain.release(device);

	vk::Device(device).destroySemaphore(imageAvailableSemaphore);
	vk::Device(device).destroySemaphore(renderFinishedSemaphore);

	device.destroy();

	instance.destroySurfaceKHR(surface);
	instance.destroy();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void AppBase::setupWindow(const bool fullscreen)
{
	if (fullscreen)
	{
		// TODO : Screen size
		width = 600;
		height = 600;
	}

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, appName, nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	uint32_t count = 0;
	const auto extensions =
		glfwGetRequiredInstanceExtensions(&count);

	for (size_t i = 0; i < count; i++)
	{
		glfwExtensions.push_back(extensions[i]);
	}

	glfwSetWindowUserPointer(window, this);
}

void AppBase::filterAvailableLayers(std::vector<const char*>& layers)
{
	auto availableLayers = vk::enumerateInstanceLayerProperties();

#if (NDEBUG)
	/* print supported layers */
	std::cout
		<< "Available validation layers: "
		<< std::endl;

	std::for_each(availableLayers.begin(),
	              availableLayers.end(),
	              [](auto const& e)
	              {
		              std::cout << e.layerName << std::endl;
	              }
	);
	std::cout << std::endl;
#endif

	std::vector<const char*> retainLayers(0);

	for (auto layer : layers)
	{
		auto found = std::find_if(availableLayers.begin(),
		                          availableLayers.end(),
		                          [layer](auto const& available)
		                          {
			                          return std::strcmp(
					                          available.layerName, layer)
				                          == 0;
		                          });
		if (found != availableLayers.end())
		{
			retainLayers.push_back(layer);
		}
	}

#if (NDEBUG)
	/* print supported layers */
	std::cout
		<< "Selected Validation layers: "
		<< std::endl;

	std::for_each(retainLayers.begin(),
	              retainLayers.end(),
	              [](auto const& e)
	              {
		              std::cout << e << std::endl;
	              }
	);

	std::cout << std::endl;
#endif

	layers.clear();

	for (auto layer : retainLayers)
	{
		layers.push_back(layer);
	}
}

void AppBase::createInstance()
{
#if (NDEBUG)
	/* print supported extensions */
	std::cout
		<< "Supported extensions: "
		<< std::endl;

	auto supportedExtensions = vk::
		enumerateInstanceExtensionProperties(nullptr);
	std::for_each(supportedExtensions.begin(),
	              supportedExtensions.end(),
	              [](auto const& e)
	              {
		              std::cout << e.extensionName << std::endl;
	              }
	);

	std::cout << std::endl;
#endif

	std::vector<const char*> instanceLayers;
	std::vector<const char*> extensions{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

#if (NDEBUG)
	instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	instanceLayers.push_back("VK_LAYER_LUNARG_monitor");
	instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	const auto applicationInfo = vk::ApplicationInfo{
		.pApplicationName = "mvk-app",
		.applicationVersion = 1,
		.pEngineName = "mvk-engine",
		.engineVersion = 1,
		.apiVersion = VK_API_VERSION_1_2,
	};

	vk::InstanceCreateInfo instanceCreateInfo = {
		.pApplicationInfo = &applicationInfo,
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};

#if (NDEBUG)
	filterAvailableLayers(instanceLayers);
	instanceCreateInfo.enabledLayerCount =
		static_cast<uint32_t>(instanceLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
#else
	{
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.ppEnabledLayerNames = nullptr;
	}
#endif

	instance = vk::createInstance(instanceCreateInfo);
}

void AppBase::createSurfaceKHR()
{
	VkSurfaceKHR _surface;
	const auto instanceVk = static_cast<VkInstance>(instance);

	if (glfwCreateWindowSurface(instanceVk, window, nullptr, &_surface)
		!= VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface khr");
	}

	surface = static_cast<vk::SurfaceKHR>(_surface);
}

void AppBase::pickPhysicalDevice()
{
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	auto physicalDevices = instance.enumeratePhysicalDevices();

	for (const auto physicalDevice : physicalDevices)
	{
		QueueFamilies queueFamilies;

		if (isPhysicalDeviceSuitable(physicalDevice,
		                             preferredQueueFamilySetting,
		                             queueFamilies))
		{
			auto supportedExtensions =
				physicalDevice.enumerateDeviceExtensionProperties(nullptr);

			std::set<std::string> requiredExtensions(deviceExtensions.begin(),
			                                         deviceExtensions.end());

			for (const auto& extension : supportedExtensions)
			{
				requiredExtensions.erase(extension.extensionName);
			}

			if (!requiredExtensions.empty())
			{
				continue;
			}

			this->physicalDevice = physicalDevice;

			graphicsQueueFamilyIndex = queueFamilies.graphicQueue.value();
			transferQueueFamilyIndex = queueFamilies.transferQueue.value();

			// Surface KHR supported ?
			if (!physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex,
			                                         surface))
			{
				throw std::runtime_error(
					"Selected device doesn't support surface KHR");
			}

#if(NDEBUG)

			std::cout
				<< "Selected physical device: "
				<< physicalDevice.getProperties().deviceName
				<< std::endl;

			std::cout
				<< "Selected graphics queue family index: "
				<< graphicsQueueFamilyIndex
				<< std::endl;

			std::cout
				<< "Selected transfer queue family index: "
				<< transferQueueFamilyIndex
				<< std::endl;

			std::cout << std::endl;
#endif

			return;
		}
	}

	throw std::runtime_error("Failed: Can't found a suitable physical device");
}

void AppBase::createDevice()
{
	device.createDevice(&physicalDevice, instance, preferredQueueFamilySetting,
	                    graphicsQueueFamilyIndex, transferQueueFamilyIndex);
}

void AppBase::createQueues()
{
	if (preferredQueueFamilySetting ==
		PreferredQueueFamilySettings::eGraphicsTransferTogether)
	{
		graphicsQueue = vk::Device(device).
			getQueue(graphicsQueueFamilyIndex, 0);
		presentQueue = vk::Device(device).getQueue(graphicsQueueFamilyIndex, 1);
		transferQueue = vk::Device(device).
			getQueue(graphicsQueueFamilyIndex, 2);
	}
	else
	{
		graphicsQueue = vk::Device(device).
			getQueue(graphicsQueueFamilyIndex, 0);
		presentQueue = vk::Device(device).getQueue(graphicsQueueFamilyIndex, 1);
		transferQueue = vk::Device(device).
			getQueue(graphicsQueueFamilyIndex, 0);
	}
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
		                                    imageAvailableSemaphore,
		                                    &imageIndex);
	}
	catch (vk::OutOfDateKHRError error)
	{
		updateSwapchain();
		buildCommandBuffers();
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

	result = graphicsQueue.submit(1, &submitInfo, nullptr);

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
		result = graphicsQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error)
	{
		updateSwapchain();
		buildCommandBuffers();
		return;
	}

	graphicsQueue.waitIdle();
}

void AppBase::setSwapchainDirty()
{
	needResize = true;
}

void AppBase::run()
{
	buildCommandBuffers();

	while (!glfwWindowShouldClose(window))
	{
		drawFrame();
		glfwPollEvents();
	}
}

void AppBase::updateSwapchain()
{
	device.waitIdle();

	swapchain.release(device);
	renderPass.release(device);

	createSwapchain();
	createRenderPass();
	createSwapchainFrames();

	needResize = false;
}

void AppBase::createSwapchain()
{
	swapchain.create(physicalDevice, device, transferQueue, surface);
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
	swapchain.createCommandBuffers(device);
}

void AppBase::setupScene()
{
	const auto size = static_cast<uint32_t>(swapchain.getSwapchainSwainSize());
	const auto extent = swapchain.getSwapchainExtent();
	scene.setup(device, size, extent);
}

void AppBase::buildCommandBuffers()
{
	auto frames = swapchain.getSwapchainFrames();

	for (const auto frame : frames)
	{
		const auto commandBuffer = frame.getCommandBuffer();
		const auto framebuffer = frame.getFramebuffer();

		const auto extent = swapchain.getSwapchainExtent();

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
				.extent = extent
			},
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data()
		};

		commandBuffer.beginRenderPass(renderPassBeginInfo,
		                              vk::SubpassContents::eInline);

		const auto objects = scene.getObjects();

		for (auto object : objects)
		{
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
				.width = static_cast<float>(extent.width),
				.height = static_cast<float>(extent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};

			commandBuffer.setViewport(0, viewport);

			vk::Rect2D scissor = {
				.offset = {0, 0},
				.extent = extent
			};

			commandBuffer.setScissor(0, scissor);

			const auto vertexBuffer = object->vertexBuffer;
			const auto indexBuffer = object->indexBuffer;

			vk::DeviceSize offsets[] = {0};

			commandBuffer.bindVertexBuffers(0, 1,
			                                &vertexBuffer.buffer,
			                                offsets);

			if (indexBuffer.buffer)
			{
				const auto size = object->indicesCount;

				commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
				                              vk::IndexType::eUint16);

				commandBuffer.drawIndexed(size, 1, 0, 0, 0);
			}
		}

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}
}

void AppBase::createSemaphores()
{
	const vk::SemaphoreCreateInfo semaphoreCreateInfo;
	imageAvailableSemaphore = vk::Device(device).createSemaphore(
		semaphoreCreateInfo);
	renderFinishedSemaphore = vk::Device(device).createSemaphore(
		semaphoreCreateInfo);
}

void AppBase::update() const
{
	const auto currentTime = std::chrono::high_resolution_clock::now();
	const auto time = std::chrono::duration<float, std::chrono::seconds::period>
		(currentTime - startTime).count();

	scene.update(device, time, swapchain.getSwapchainExtent());
}

void AppBase::framebufferResizeCallback(GLFWwindow* window, int width,
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
