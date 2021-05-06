#include "SwapchainFrame.h"

using namespace mvk;

void SwapchainFrame::create(Device* device,
                            const vk::Image image,
                            const vk::RenderPass renderPass,
                            const vk::Format swapchainFormat,
                            const vk::Extent2D swapchainExtent,
                            const vk::ImageView depthImage)
{
	this->ptrDevice = device;

	createImageView(image, swapchainFormat);
	createFramebuffer(depthImage, renderPass, swapchainExtent);
}

void SwapchainFrame::release() const
{
	ptrDevice->logicalDevice.destroy(framebuffer);
	ptrDevice->logicalDevice.destroy(imageView);
}

void SwapchainFrame::createImageView(vk::Image image, vk::Format format)
{
	const auto imageViewCreateInfo = vk::ImageViewCreateInfo{
		.image = image,
		.viewType = vk::ImageViewType::e2D,
		.format = format,
		.components = {
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity,
		},
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	imageView = ptrDevice->logicalDevice.createImageView(imageViewCreateInfo);
}

void SwapchainFrame::createFramebuffer(const vk::ImageView depthImageView,
                                       const vk::RenderPass renderPass,
                                       const vk::Extent2D extent)
{
	std::array<vk::ImageView, 2> attachments = {imageView, depthImageView};

	const vk::FramebufferCreateInfo framebufferCreateInfo = {
		.renderPass = renderPass,
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.width = extent.width,
		.height = extent.height,
		.layers = 1
	};

	framebuffer =
		ptrDevice->logicalDevice.createFramebuffer(framebufferCreateInfo);
}
