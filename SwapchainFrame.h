#pragma once
#include "VulkanVma.h"

namespace mvk
{
	class SwapchainFrame
	{
		vk::Format format;
		vk::Extent2D extent;

		vk::Device device;
		vma::Allocator allocator;

		vk::Image image;
		vk::ImageView imageView;
		vk::Framebuffer framebuffer;
		vk::CommandBuffer commandBuffer;

		AllocatedBuffer uniformBuffer;
		vk::DescriptorSet descriptorSet;

		vk::RenderPass renderPass;

		void createImageView(vk::Image image);
		void createFramebuffer(vk::ImageView depthImageView);
		void createUniformBuffer();

	public:
		void init(vk::Device device,
		          vk::Image image,
		          vma::Allocator allocator,
		          vk::RenderPass renderPass,
		          vk::Format swapchainFormat,
		          vk::Extent2D swapchainExtent,
		          vk::ImageView depthImage);
		void release();

		vk::Image getImage() const { return image; }
		vk::ImageView getImageView() const { return imageView; }
		vk::Framebuffer getFramebuffer() const { return framebuffer; }
		AllocatedBuffer getUniformBuffer() const { return uniformBuffer; }
		vk::CommandBuffer getCommandBuffer() const { return commandBuffer; }
		vk::DescriptorSet getDescriptorSet() const { return descriptorSet; }

		void setCommandBuffer(const vk::CommandBuffer commandBuffer)
		{
			this->commandBuffer = commandBuffer;
		}

		void setDescriptorSet(const vk::DescriptorSet descriptorSet)
		{
			this->descriptorSet = descriptorSet;
		}
	};
}
