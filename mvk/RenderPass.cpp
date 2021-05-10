#include "RenderPass.h"

using namespace mvk;

void RenderPass::create(Device* device, const vk::Format colorFormat,
                        const vk::Format depthFormat)
{
	this->ptrDevice = device;

	/** Color Description **/
	const vk::AttachmentDescription colorAttachment{
		.format = colorFormat,
		.samples = ptrDevice->multiSampling,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::eColorAttachmentOptimal
	};

	/** Depth description **/
	const vk::AttachmentDescription depthAttachment{
		.format = depthFormat,
		.samples = ptrDevice->multiSampling,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
	};

	/*Resolve Color description*/
	const vk::AttachmentDescription colorAttachmentResolve{
		.format = colorFormat,
		.samples = vk::SampleCountFlagBits::e1,
		.loadOp = vk::AttachmentLoadOp::eDontCare,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
		.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
		.initialLayout = vk::ImageLayout::eUndefined,
		.finalLayout = vk::ImageLayout::ePresentSrcKHR
	};

	/** SubPass and attachments references **/
	const vk::AttachmentReference colorReference{
		.attachment = 0,
		.layout = vk::ImageLayout::eColorAttachmentOptimal
	};

	const vk::AttachmentReference depthReference{
		.attachment = 1,
		.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
	};

	const vk::AttachmentReference colorReferenceResolve{
		.attachment = 2,
		.layout = vk::ImageLayout::eColorAttachmentOptimal
	};

	const vk::SubpassDescription subPass{
		.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorReference,
		.pResolveAttachments = &colorReferenceResolve,
		.pDepthStencilAttachment = &depthReference
	};

	const std::array<vk::AttachmentDescription, 3> attachments{
		colorAttachment,
		depthAttachment,
		colorAttachmentResolve
	};

	const auto attachmentCount = static_cast<uint32_t>(attachments.size());

	const vk::RenderPassCreateInfo renderPassCreateInfo{
		.attachmentCount = attachmentCount,
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.pSubpasses = &subPass
	};

	renderPass = device->logicalDevice.createRenderPass(renderPassCreateInfo);
}

void RenderPass::release() const
{
	if (ptrDevice != nullptr)
	{
		ptrDevice->logicalDevice.destroyRenderPass(renderPass);
	}
}
