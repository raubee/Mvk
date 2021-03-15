#include "SwapchainFrame.h"
#include "UniformBufferObject.h"

using namespace mvk;

void SwapchainFrame::init(const vk::Device device,
                          const vk::Image image,
                          const vma::Allocator allocator,
                          const vk::RenderPass renderPass,
                          const vk::Format swapchainFormat,
                          const vk::Extent2D swapchainExtent,
                          const vk::ImageView depthImage)
{
	this->device = device;
	this->allocator = allocator;
	this->renderPass = renderPass;
	this->image = image;
	this->format = swapchainFormat;
	this->extent = swapchainExtent;

	createImageView(image);
	createFramebuffer(depthImage);
	createUniformBuffer();
}

void SwapchainFrame::release()
{
	deallocateBuffer(allocator, uniformBuffer);

	if (framebuffer)
		device.destroy(framebuffer);

	if (imageView)
		device.destroy(imageView);
}

void SwapchainFrame::createImageView(vk::Image image)
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

	imageView = device.createImageView(imageViewCreateInfo);
}

void SwapchainFrame::createFramebuffer(const vk::ImageView depthImageView)
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

	framebuffer = device.createFramebuffer(framebufferCreateInfo);
}

void SwapchainFrame::createUniformBuffer()
{
	const auto size = sizeof(UniformBufferObject);

	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eUniformBuffer,
	};

	uniformBuffer = allocateCpuToGpuBuffer(allocator, bufferCreateInfo);
}
