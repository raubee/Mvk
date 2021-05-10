#pragma once

#include "VulkanVma.h"
#include "Device.hpp"

namespace mvk
{
	class SwapchainFrame
	{
		Device* ptrDevice;

		vk::ImageView imageView;
		vk::Framebuffer framebuffer;
		vk::CommandBuffer commandBuffer;

		void createImageView(vk::Image image, vk::Format format);

		void createFramebuffer(vk::ImageView colorImageView,
		                       vk::ImageView depthImageView,
		                       vk::RenderPass renderPass,
		                       vk::Extent2D extent);

	public:

		void create(Device* device,
		            vk::Image image,
		            vk::RenderPass renderPass,
		            vk::Format swapchainFormat,
		            vk::Extent2D swapchainExtent,
		            vk::ImageView colorImageView,
		            vk::ImageView depthImageView);

		void release() const;

		vk::ImageView getImageView() const { return imageView; }
		vk::Framebuffer getFramebuffer() const { return framebuffer; }
		vk::CommandBuffer getCommandBuffer() const { return commandBuffer; }

		void setCommandBuffer(const vk::CommandBuffer commandBuffer)
		{
			this->commandBuffer = commandBuffer;
		}
	};
}
