#include "SwapChain.h"
#include "UniformBufferObject.h"

using namespace mvk;

void SwapChain::create(const vk::PhysicalDevice physicalDevice,
                       const Device device,
                       const vk::Queue transferQueue,
                       const vk::SurfaceKHR surface)
{
	createSwapChainKHR(physicalDevice, device, surface);
	createDepthImageView(device, transferQueue);
}

void SwapChain::createSwapChainKHR(const vk::PhysicalDevice physicalDevice,
                                   const mvk::Device device,
                                   const vk::SurfaceKHR surface)
{
	uint32_t queueFamilyIndices = 0;
	const auto presentMode =
		selectSwapchainPresentMode(physicalDevice, surface);
	const auto capabilities = getSwapchainCapabilities(physicalDevice, surface);

	size = capabilities.capabilities.minImageCount + 1;
	extent = capabilities.capabilities.currentExtent;
	frameFormat = selectSwapchainFormat(capabilities.formats);

	const vk::SwapchainCreateInfoKHR swapchainCreateInfoKhr = {
		.surface = surface,
		.minImageCount = size,
		.imageFormat = frameFormat,
		.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.pQueueFamilyIndices = &queueFamilyIndices,
		.preTransform = capabilities.capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
	};

	swapchain = vk::Device(device).createSwapchainKHR(swapchainCreateInfoKhr);
}

void SwapChain::createCommandBuffers(const mvk::Device device)
{
	const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {
		.commandPool = vk::CommandPool(device),
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = static_cast<uint32_t>(size)
	};

	commandBuffers =
		vk::Device(device).allocateCommandBuffers(commandBufferAllocateInfo);

	auto i = 0;
	for (const auto commandBuffer : commandBuffers)
	{
		swapchainFrames[i].setCommandBuffer(commandBuffer);
		i++;
	}
}

void SwapChain::createSwapchainFrames(const Device device,
                                      const vk::RenderPass renderPass)
{
	swapchainFrames.resize(size);
	const auto swapchainImages =
		vk::Device(device).getSwapchainImagesKHR(swapchain);

	for (auto i = 0; i < swapchainFrames.size(); i++)
	{
		swapchainFrames[i].create(device, swapchainImages[i], renderPass,
		                          frameFormat, extent,
		                          depthImageView);
	}
}

void SwapChain::release(const Device device)
{
	for (auto swapchainFrame : swapchainFrames)
	{
		swapchainFrame.release(device);
	}

	vk::Device(device).destroyImageView(depthImageView);
	device.destroyImage(depthImage);

	vk::Device(device).destroySwapchainKHR(swapchain);
}

void SwapChain::createDepthImageView(const Device device,
                                     const vk::Queue transferQueue)
{
	depthFormat = vk::Format::eD32Sfloat;

	const vk::ImageCreateInfo imageCreateInfo = {
		.imageType = vk::ImageType::e2D,
		.format = depthFormat,
		.extent = {
			.width = extent.width,
			.height = extent.height,
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

	depthImage = mvk::alloc::allocateGpuOnlyImage(
		vma::Allocator(device), imageCreateInfo);

	device.transitionImageLayout(transferQueue, depthImage.image,
	                             vk::ImageLayout::eUndefined,
	                             vk::ImageLayout::
	                             eDepthStencilAttachmentOptimal);

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

	depthImageView = vk::Device(device).createImageView(imageViewCreateInfo);
}

SurfaceCapabilitiesKHRBatch SwapChain::getSwapchainCapabilities(
	const vk::PhysicalDevice physicalDevice,
	const vk::SurfaceKHR surface) const
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

vk::PresentModeKHR SwapChain::selectSwapchainPresentMode(
	const vk::PhysicalDevice physicalDevice,
	const vk::SurfaceKHR surface) const
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
