#include "SwapChain.h"
#include "UniformBufferObject.h"

using namespace mvk;

void SwapChain::create(const vk::PhysicalDevice physicalDevice,
                       const vk::Device device,
                       const vma::Allocator allocator,
                       const vk::CommandPool commandPool,
                       const vk::Queue transferQueue,
                       const vk::SurfaceKHR surface)
{
	createSwapChainKHR(physicalDevice, device, surface);
	createDepthImageView(device, allocator, commandPool, transferQueue);
}

void SwapChain::createSwapChainKHR(const vk::PhysicalDevice physicalDevice,
                                   const vk::Device device,
                                   const vk::SurfaceKHR surface)
{
	uint32_t queueFamilyIndices = 0;
	const auto presentMode =
		selectSwapchainPresentMode(physicalDevice, surface);
	const auto capabilities = getSwapchainCapabilities(physicalDevice, surface);

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

void SwapChain::createCommandBuffers(const vk::Device device,
                                     const vk::CommandPool commandPool)
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

void SwapChain::createSwapchainFrames(const vk::Device device,
                                      const vk::RenderPass renderPass)
{
	swapchainFrames.resize(swapchainSize);
	const auto swapchainImages = device.getSwapchainImagesKHR(swapchain);

	for (auto i = 0; i < swapchainFrames.size(); i++)
	{
		swapchainFrames[i].create(device, swapchainImages[i], renderPass,
		                          swapchainFormat, swapchainExtent,
		                          depthImageView);
	}
}

void SwapChain::release(const vk::Device device, const vma::Allocator allocator)
{
	for (auto swapchainFrame : swapchainFrames)
	{
		swapchainFrame.release(device);
	}

	if (depthImageView)
		device.destroyImageView(depthImageView);

	if (depthImage.image)
		mvk::alloc::deallocateImage(allocator, depthImage);

	if (swapchain)
		device.destroySwapchainKHR(swapchain);
}

void SwapChain::createDepthImageView(const vk::Device device,
                                     const vma::Allocator allocator,
                                     const vk::CommandPool commandPool,
                                     const vk::Queue transferQueue)
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

	depthImage = mvk::alloc::allocateGpuOnlyImage(allocator, imageCreateInfo);
	mvk::alloc::transitionImageLayout(device, commandPool, transferQueue,
	                                  depthImage.image, depthFormat,
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

	depthImageView = device.createImageView(imageViewCreateInfo);
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
