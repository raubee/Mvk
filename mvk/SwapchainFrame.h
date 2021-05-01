#pragma once

#include "VulkanVma.h"
#include "Device.hpp"

namespace mvk
{
	class SwapchainFrame
	{
		vk::ImageView imageView;
		vk::Framebuffer framebuffer;
		vk::CommandBuffer commandBuffer;

		void createImageView(Device device,
		                     vk::Image image,
		                     vk::Format format);

		void createFramebuffer(Device device,
		                       vk::ImageView depthImageView,
		                       vk::RenderPass renderPass,
		                       vk::Extent2D extent);

	public:

		void create(Device device,
		            vk::Image image,
		            vk::RenderPass renderPass,
		            vk::Format swapchainFormat,
		            vk::Extent2D swapchainExtent,
		            vk::ImageView depthImage);

		void release(Device device) const;

		vk::ImageView getImageView() const { return imageView; }
		vk::Framebuffer getFramebuffer() const { return framebuffer; }
		vk::CommandBuffer getCommandBuffer() const { return commandBuffer; }

		void setCommandBuffer(const vk::CommandBuffer commandBuffer)
		{
			this->commandBuffer = commandBuffer;
		}
	};
}
