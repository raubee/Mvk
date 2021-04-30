#include "RenderPass.h"

using namespace mvk;

void RenderPass::create(const vk::Device device,
                        const vk::Format colorFormat,
                        const vk::Format depthFormat)
{
	/** Color Description **/
	const vk::AttachmentDescription colorAttachment = {
		.format = colorFormat,
		.samples = vk::SampleCountFlagBits::e1,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::ePresentSrcKHR
	};

	/** Depth description **/
	const vk::AttachmentDescription depthAttachment = {
		.format = depthFormat,
		.samples = vk::SampleCountFlagBits::e1,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
	};

	/** SubPass and attachments references **/
	const vk::AttachmentReference colorReference = {
		.attachment = 0,
		.layout = vk::ImageLayout::eColorAttachmentOptimal
	};

	const vk::AttachmentReference depthReference = {
		.attachment = 1,
		.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
	};

	const vk::SubpassDescription subPass = {
		.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorReference,
		.pDepthStencilAttachment = &depthReference
	};

	const std::array<vk::AttachmentDescription, 2> attachments = {
		colorAttachment,
		depthAttachment
	};

	const vk::RenderPassCreateInfo renderPassCreateInfo = {
		.attachmentCount = static_cast<uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subPass
	};

	renderPass = device.createRenderPass(renderPassCreateInfo);
}

void RenderPass::release(const vk::Device device) const
{
	device.destroyRenderPass(renderPass);
}
