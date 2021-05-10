#include "SwapchainFrame.h"

using namespace mvk;

void SwapchainFrame::create(Device* device,
                            const vk::Image image,
                            const vk::RenderPass renderPass,
                            const vk::Format swapchainFormat,
                            const vk::Extent2D swapchainExtent,
                            const vk::ImageView colorImageView,
                            const vk::ImageView depthImageView)
{
	this->ptrDevice = device;

	createImageView(image, swapchainFormat);
	createFramebuffer(colorImageView, depthImageView, renderPass,
	                  swapchainExtent);
}

void SwapchainFrame::release() const
{
	ptrDevice->logicalDevice.destroy(framebuffer);
	ptrDevice->logicalDevice.destroy(imageView);
}

void SwapchainFrame::createImageView(const vk::Image image,
                                     const vk::Format format)
{
	const vk::ImageViewCreateInfo imageViewCreateInfo{
		.image = image,
		.viewType = vk::ImageViewType::e2D,
		.format = format,
		.components{
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity,
		},
		.subresourceRange{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
	};

	imageView = ptrDevice->logicalDevice.createImageView(imageViewCreateInfo);
}

void SwapchainFrame::createFramebuffer(const vk::ImageView colorImageView,
                                       const vk::ImageView depthImageView,
                                       const vk::RenderPass renderPass,
                                       const vk::Extent2D extent)
{
	std::array<vk::ImageView, 3> attachments =
		{colorImageView, depthImageView, imageView};

	const auto attachmentCount = static_cast<uint32_t>(attachments.size());

	const vk::FramebufferCreateInfo framebufferCreateInfo{
		.renderPass = renderPass,
		.attachmentCount = attachmentCount,
		.pAttachments = attachments.data(),
		.width = extent.width,
		.height = extent.height,
		.layers = 1
	};

	framebuffer =
		ptrDevice->logicalDevice.createFramebuffer(framebufferCreateInfo);
}
