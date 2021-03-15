#include "SwapChain.h"
#include "UniformBufferObject.h"

using namespace mvk;

void SwapChain::init(const vk::PhysicalDevice physicalDevice,
                     const vk::Device device,
                     const vk::CommandPool commandPool,
                     const vma::Allocator allocator,
                     const vk::Queue transferQueue,
                     const vk::SurfaceKHR surface)
{
	this->physicalDevice = physicalDevice;
	this->device = device;
	this->commandPool = commandPool;
	this->allocator = allocator;
	this->transferQueue = transferQueue;
	this->surface = surface;

	createSwapchain();
	createDepthImageView();
}

void SwapChain::createSwapchainFramesCommandBuffers()
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {
		.commandPool = commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = static_cast<uint32_t>(swapchainSize)
	};

	commandBuffers = device.allocateCommandBuffers(commandBufferAllocateInfo);

	auto i = 0;
	for (const auto commandBuffer : commandBuffers)
	{
		swapchainFrames[i].setCommandBuffer(commandBuffer);
		i++;
	}
}

void SwapChain::createDescriptorPool()
{
	vk::DescriptorPoolSize descriptorPoolSize{
		.type = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = static_cast<uint32_t>(swapchainSize)
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		.maxSets = swapchainSize,
		.poolSizeCount = 1,
		.pPoolSizes = &descriptorPoolSize
	};

	descriptorPool = device.createDescriptorPool(descriptorPoolCreateInfo);
}

void SwapChain::createDescriptorSets(
	const vk::DescriptorSetLayout descriptorSetLayout)
{
	createDescriptorPool();

	std::vector<vk::DescriptorSetLayout> layouts(swapchainSize,
	                                             descriptorSetLayout);

	const vk::DescriptorSetAllocateInfo allocateInfo{
		.descriptorPool = descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(swapchainSize),
		.pSetLayouts = layouts.data()
	};

	descriptorSets = device.allocateDescriptorSets(allocateInfo);

	auto i = 0;
	for (const auto descriptorSet : descriptorSets)
	{
		const vk::DescriptorBufferInfo descriptorBufferInfo = {
			.buffer = swapchainFrames[i].getUniformBuffer().buffer,
			.offset = 0,
			.range = sizeof(UniformBufferObject)
		};

		const vk::WriteDescriptorSet writeDescriptorSet = {
			.dstSet = descriptorSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &descriptorBufferInfo
		};

		device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
		swapchainFrames[i].setDescriptorSet(descriptorSet);
		i++;
	}
}

void SwapChain::createSwapchainFrames(const vk::RenderPass renderPass)
{
	swapchainFrames.resize(swapchainSize);
	const auto swapchainImages = device.getSwapchainImagesKHR(swapchain);

	for (auto i = 0; i < swapchainFrames.size(); i++)
	{
		swapchainFrames[i].init(device, swapchainImages[i], allocator,
		                        renderPass, swapchainFormat, swapchainExtent,
		                        depthImageView);
	}
}

void SwapChain::release()
{
	for (auto swapchainFrame : swapchainFrames)
	{
		swapchainFrame.release();
	}

	if (depthImageView)
		device.destroyImageView(depthImageView);
	
	if (depthImage.image)
		deallocateImage(allocator, depthImage);

	if (descriptorPool)
		device.destroyDescriptorPool(descriptorPool);

	if (swapchain)
		device.destroySwapchainKHR(swapchain);
}

void SwapChain::createSwapchain()
{
	uint32_t queueFamilyIndices = 0;
	const auto presentMode = selectSwapchainPresentMode();
	const auto capabilities = getSwapchainCapabilities();

	swapchainSize = capabilities.capabilities.minImageCount + 1;
	swapchainExtent = capabilities.capabilities.currentExtent;
	swapchainFormat = selectSwapchainFormat(capabilities.formats);

	const vk::SwapchainCreateInfoKHR swapchainCreateInfoKhr = {
		.surface = surface,
		.minImageCount = swapchainSize,
		.imageFormat = swapchainFormat,
		.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
		.imageExtent = swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.pQueueFamilyIndices = &queueFamilyIndices,
		.preTransform = capabilities.capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
	};

	swapchain = device.createSwapchainKHR(swapchainCreateInfoKhr);
}

void SwapChain::createDepthImageView()
{
	// TODO: Best appropriated format supported by the physical device
	depthFormat = vk::Format::eD32Sfloat;

	const vk::ImageCreateInfo imageCreateInfo = {
		.imageType = vk::ImageType::e2D,
		.format = depthFormat,
		.extent = {
			.width = swapchainExtent.width,
			.height = swapchainExtent.height,
			.depth = 1,
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined
	};

	depthImage = allocateGpuOnlyImage(allocator, imageCreateInfo);
	transitionImageLayout(allocator, device, commandPool, transferQueue,
	                      depthImage.image, depthFormat,
	                      vk::ImageLayout::eUndefined,
	                      vk::ImageLayout::eDepthStencilAttachmentOptimal);

	const vk::ImageViewCreateInfo imageViewCreateInfo = {
		.image = depthImage.image,
		.viewType = vk::ImageViewType::e2D,
		.format = depthFormat,
		.components = {
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity,
		},
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eDepth,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
	};

	depthImageView = device.createImageView(imageViewCreateInfo);
}

SurfaceCapabilitiesKHRBatch SwapChain::getSwapchainCapabilities() const
{
	const auto capabilities =
		physicalDevice.getSurfaceCapabilitiesKHR(surface);

	const auto formats =
		physicalDevice.getSurfaceFormatsKHR(surface);

	return SurfaceCapabilitiesKHRBatch{capabilities, formats};
}

vk::Format SwapChain::selectSwapchainFormat(
	std::vector<vk::SurfaceFormatKHR> formats) const
{
	const auto f = std::find_if(
		formats.begin(), formats.end(), [](vk::SurfaceFormatKHR const& f)
		{
			return
				f.format == vk::Format::eB8G8R8A8Srgb &&
				f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});

	return (*f).format;
}

vk::PresentModeKHR SwapChain::selectSwapchainPresentMode() const
{
	auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	const auto mode = std::find_if(presentModes.begin(), presentModes.end(),
	                               [](vk::PresentModeKHR const& p)
	                               {
		                               return p == vk::PresentModeKHR::
			                               eMailbox;
	                               });

	if (mode == presentModes.end())
	{
		return vk::PresentModeKHR::eFifo;
	}

	return *mode;
}
